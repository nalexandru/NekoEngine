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

//#include "MacXPlatform.h"

#import <UIKit/UIKit.h>

#include "EngineAppDelegate.h"

#define INFO_STR_LEN	128

extern natural_t Darwin_numCpus;
extern int32_t Darwin_cpuFreq;
extern struct utsname Darwin_uname;
extern char Darwin_osName[INFO_STR_LEN];
extern char Darwin_osVersionString[INFO_STR_LEN];
extern char Darwin_cpuName[INFO_STR_LEN];
extern NSURL *Darwin_appSupportURL;

bool Sys_InitDarwinPlatform(void);
void Sys_TermDarwinPlatform(void);

enum NeMachineType
Sys_MachineType(void)
{
	return MT_MOBILE;
}

void
Sys_MessageBox(const char *title, const char *message, int icon)
{
	@autoreleasepool {
		UIAlertController *ctl = [UIAlertController alertControllerWithTitle: [NSString stringWithUTF8String: title]
																	 message: [NSString stringWithUTF8String: message]
															  preferredStyle: UIAlertControllerStyleAlert];

		//__block bool dismissed = false;
		UIAlertAction *act = [UIAlertAction actionWithTitle: @"OK" style: UIAlertActionStyleDefault handler: ^(UIAlertAction * _Nonnull action) {
			//dismissed = true;
		}];
		
		[ctl addAction: act];
		
		[[(UIWindow *)E_screen rootViewController] presentViewController: ctl animated: true completion: nil];
		
		[ctl dismissViewControllerAnimated: true completion: nil];
	}
}

bool
Sys_ProcessEvents(void)
{
	return true;
}

bool
Sys_InitPlatform(void)
{
	@autoreleasepool {
		processor_basic_info_t cpuInfo;
		mach_msg_type_number_t msgCount;
		host_processor_info(mach_host_self(), PROCESSOR_BASIC_INFO, &Darwin_numCpus, (processor_info_array_t *)&cpuInfo, &msgCount);
	
		if (!Sys_InitDarwinPlatform())
			return false;
		
		UIDevice *dev = [UIDevice currentDevice];

		snprintf(Darwin_osName, sizeof(Darwin_osName), "%s (%s)", Darwin_uname.sysname, [[dev systemName] UTF8String]);
		snprintf(Darwin_osVersionString, sizeof(Darwin_osVersionString), "%s (%s)", Darwin_uname.release, [[dev systemVersion] UTF8String]);
		
		int mib[2] = { CTL_HW, HW_MODEL };
		char cpu[INFO_STR_LEN / 2], model[INFO_STR_LEN / 2];
		size_t len = sizeof(cpu);
		sysctl(mib, 2, cpu, &len, NULL, 0);
		mib[1] = HW_MACHINE;
		len = sizeof(model);
		sysctl(mib, 2, model, &len, NULL, 0);
	
		snprintf(Darwin_cpuName, sizeof(Darwin_cpuName), "%s (%s)", model, cpu);
		
		// FIXME
		if (strstr(Darwin_cpuName, "iPhone10"))
			Darwin_cpuFreq = 2390.f;
		else if (strstr(Darwin_cpuName, "iPhone11"))
			Darwin_cpuFreq = 2490.f;
		else if (strstr(Darwin_cpuName, "iPhone12"))
			Darwin_cpuFreq = 2650.f;
		else if (strstr(Darwin_cpuName, "iPhone13"))
			Darwin_cpuFreq = 3100.f;
		
		return true;
	}
}

void
Sys_TermPlatform(void)
{
	Sys_TermDarwinPlatform();
}

intptr_t
Sys_GetCurrentProcess()
{
	return 0;
}

int32_t
Sys_GetCurrentProcessId()
{
	return 0;
}

void
Sys_WaitForProcessExit(intptr_t handle)
{
}

intptr_t
Sys_Execute(const char **argv, const char *envp, const char *wd, FILE **in, FILE **out, FILE **err, bool showWindow)
{
	return -1;
}

bool
Sys_TerminateProcess(intptr_t handle)
{
	return false;
}

/* NekoEngine
 *
 * iOS.m
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
