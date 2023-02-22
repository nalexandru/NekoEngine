#include <stdio.h>
#include <stdint.h>

#include <sched.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/host_info.h>
#include <mach/processor_info.h>

#include <Input/Input.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Engine/Application.h>

#include "macOSPlatform.h"

#import <Cocoa/Cocoa.h>

#include "EngineAppDelegate.h"

#define INFO_STR_LEN	128

extern natural_t Darwin_numCpus;
extern int32_t Darwin_cpuFreq;
extern struct utsname Darwin_uname;
extern char Darwin_osName[INFO_STR_LEN];
extern char Darwin_osVersionString[INFO_STR_LEN];
extern char Darwin_cpuName[INFO_STR_LEN];
extern NSURL *Darwin_appSupportURL;

static inline void _CpuInfo(void);

bool Sys_InitDarwinPlatform(void);
void Sys_TermDarwinPlatform(void);

enum NeMachineType
Sys_MachineType(void)
{
	return MT_PC;
}

void
Sys_MessageBox(const char *title, const char *message, int icon)
{
	NSAlert *a = [[NSAlert alloc] init];
	[a addButtonWithTitle:@"OK"];
	[a setMessageText: [NSString stringWithUTF8String: message]];
	
	switch (icon) {
	case MSG_ICON_NONE: break;
	case MSG_ICON_INFO: [a setAlertStyle: NSAlertStyleInformational]; break;
	case MSG_ICON_WARN: [a setAlertStyle: NSAlertStyleWarning]; break;
	case MSG_ICON_ERROR: [a setAlertStyle: NSAlertStyleCritical]; break;
	}
	
	[a runModal];
	[a release];
}

bool
Sys_ProcessEvents(void)
{
	@autoreleasepool {
		while (1) {
			NSEvent *e = [NSApp nextEventMatchingMask: NSEventMaskAny
											untilDate: [NSDate distantPast]
											   inMode: NSDefaultRunLoopMode
											  dequeue: true];

			if (!e)
				break;

			[NSApp sendEvent: e];
		}
	}

	return true;
}

bool
Sys_InitPlatform(void)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSApplicationLoad();
	[NSApplication sharedApplication];
	[NSApp finishLaunching];
	
	[NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];

	EngineAppDelegate *d = [[EngineAppDelegate alloc] init];
	[NSApp setDelegate: d];
	[[NSApplication sharedApplication] setDelegate: d];

	NSMenu *mainMenu = [[[NSMenu alloc] init] autorelease];
	{
		NSString *appName = [NSString stringWithUTF8String: App_applicationInfo.name];
		NSMenu *appMenu = [[[NSMenu alloc] init] autorelease];

		NSMenuItem *menuItem = [[[NSMenuItem alloc] init] autorelease];
		[menuItem setTitle: [NSString stringWithFormat: @"About %@", appName]];
		[menuItem setAction: @selector(about:)];
		[menuItem setTarget: d];
		[appMenu addItem: menuItem];

		[appMenu addItem: [NSMenuItem separatorItem]];

		menuItem = [[[NSMenuItem alloc] init] autorelease];
		[menuItem setTitle: [NSString stringWithFormat: @"Quit %@", appName]];
		[menuItem setAction: @selector(quit:)];
		[menuItem setTarget: d];
		[appMenu addItem: menuItem];

		menuItem = [[NSMenuItem alloc] init];
		[menuItem setSubmenu: appMenu];
		[mainMenu addItem: menuItem];
	}
	[NSApp setMainMenu: mainMenu];

	if (!Sys_InitDarwinPlatform())
		return false;
	
	_CpuInfo();
	
	snprintf(Darwin_osName, sizeof(Darwin_osName), "%s (macOS)", Darwin_uname.sysname);
	snprintf(Darwin_osVersionString, sizeof(Darwin_osVersionString), "%s (%s)", Darwin_uname.release,
				[[[NSProcessInfo processInfo] operatingSystemVersionString] UTF8String]);

	if (!Sys_DirectoryExists("Data"))
		E_SetCVarStr("Engine_DataDir", [[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent: @"Data"] UTF8String]);
	
	[pool drain];
	
	return true;
}

void
Sys_TermPlatform(void)
{
	Sys_TermDarwinPlatform();
}

intptr_t
Sys_GetCurrentProcess()
{
	return (intptr_t)getpid();
}

int32_t
Sys_GetCurrentProcessId()
{
	return getpid();
}

void
Sys_WaitForProcessExit(intptr_t handle)
{
	waitpid((pid_t)handle, NULL, 0);
}

intptr_t
Sys_Execute(char * const *argv, const char *wd, FILE **in, FILE **out, FILE **err, bool showWindow)
{
	int inPipes[2], outPipes[2], errPipes[2];

	if (in)
		if (pipe(inPipes))
			return -1;

	if (out)
		if (pipe(outPipes))
			return -1;

	if (err)
		if (pipe(errPipes))
			return -1;

	pid_t p = fork();
	if (!p) {
		if (in) {
			close(inPipes[1]);
			dup2(inPipes[0], STDIN_FILENO);
		}

		if (out) {
			close(outPipes[0]);
			dup2(outPipes[1], STDOUT_FILENO);
		}

		if (err) {
			close(errPipes[0]);
			dup2(errPipes[1], STDERR_FILENO);
		}

		if (wd)
			chdir(wd);

		execv(argv[0], argv);
	} else {
		if (in) {
			close(inPipes[0]);
			*in = fdopen(inPipes[1], "w");
		}

		if (out) {
			close(outPipes[1]);
			*out = fdopen(outPipes[0], "r");
		}

		if (err) {
			close(errPipes[1]);
			*err = fdopen(errPipes[0], "r");
		}
	}

	return (intptr_t)p;
}

bool
Sys_TerminateProcess(intptr_t handle)
{
	return !kill((pid_t)handle, SIGTERM);
}

void
_CpuInfo(void)
{
	processor_basic_info_t cpuInfo;
	mach_msg_type_number_t msgCount;
	
	host_processor_info(mach_host_self(), PROCESSOR_BASIC_INFO, &Darwin_numCpus,
						(processor_info_array_t *)&cpuInfo, &msgCount);
	
	size_t len = sizeof(Darwin_cpuName);
	sysctlbyname("machdep.cpu.brand_string", Darwin_cpuName, &len, NULL, 0);
	
#ifdef __arm64
	// FIXME
	if (strstr(Darwin_cpuName, "M1"))
		Darwin_cpuFreq = 3200.f;
#else
	int null;
	FILE *fp = NULL;
	char buff[512], *p = buff;
	bool ghz = false;
	float f = 0.f;
	static int sout, serr;
	
	// slience stdout, stderr
	fflush(stdout);
	fflush(stderr);
	
	null = open("/dev/null", O_WRONLY);
	
	sout = dup(STDOUT_FILENO);
	dup2(null, STDOUT_FILENO);
	
	serr = dup(STDERR_FILENO);
	dup2(null, STDERR_FILENO);
	
	close(null);
	
	fp = popen("system_profiler SPHardwareDataType | grep 'Processor Speed' | cut -c 24-", "r");
	fgets(buff, sizeof(buff), fp);
	pclose(fp);
	
	while (*(p++)) {
		if (*p == ',')
			*p = '.';
		
		if (*p != ' ')
			continue;
			
		*p = 0x0;
		if (*(++p) == 'G')
			ghz = true;
			
		break;
	}
	
	f = atof(buff) * (ghz ? 1000.f : 1.f);
	Darwin_cpuFreq = (int32_t)f;
	
	// restore stdout, stderr
	fflush(stdout);
	dup2(sout, STDOUT_FILENO);
	
	fflush(stderr);
	dup2(serr, STDERR_FILENO);
#endif
}

/* NekoEngine
 *
 * macOS.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
