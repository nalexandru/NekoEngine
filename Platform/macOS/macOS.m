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
