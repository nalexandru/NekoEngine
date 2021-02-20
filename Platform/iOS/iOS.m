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

#define Handle __EngineHandle

#include <Input/Input.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Engine/Application.h>

//#include "MacXPlatform.h"

#undef Handle
#import <UIKit/UIKit.h>

#include "EngineAppDelegate.h"

#define INFO_STR_LEN	128

extern natural_t Darwin_numCpus;
extern int32_t Darwin_cpuFreq;
extern struct utsname Darwin_uname;
extern char Darwin_osName[INFO_STR_LEN];
extern char Darwin_osVersion[INFO_STR_LEN];
extern char Darwin_cpuName[INFO_STR_LEN];
extern NSURL *Darwin_appSupportURL;

bool Sys_InitDarwinPlatform(void);
void Sys_TermDarwinPlatform(void);

enum MachineType
Sys_MachineType(void)
{
	return MT_MOBILE;
}

void
Sys_MessageBox(const wchar_t *title, const wchar_t *message, int icon)
{
	char *t = Sys_Alloc(wcslen(title), sizeof(*t) + 1, MH_Transient);
	wcstombs(t, title, wcslen(title));
	
	char *m = Sys_Alloc(wcslen(message), sizeof(*m) + 1, MH_Transient);
	wcstombs(m, message, wcslen(message));
	
	@autoreleasepool {
		UIAlertController *ctl = [UIAlertController alertControllerWithTitle: [NSString stringWithFormat: @"%s", t]
																	 message: [NSString stringWithFormat: @"%s", m]
															  preferredStyle: UIAlertControllerStyleAlert];

		UIAlertAction *act = [UIAlertAction actionWithTitle: @"OK" style: UIAlertActionStyleDefault handler: nil];
		
		[ctl addAction: act];
		
		[[(UIWindow *)E_screen rootViewController] presentViewController: ctl animated: true completion: nil];
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
		host_processor_info(mach_host_self(), PROCESSOR_BASIC_INFO, &Darwin_numCpus,
							(processor_info_array_t *)&cpuInfo, &msgCount);
	
		if (!Sys_InitDarwinPlatform())
			return false;
		
		UIDevice *dev = [UIDevice currentDevice];

		snprintf(Darwin_osName, sizeof(Darwin_osName), "%s (%s)", Darwin_uname.sysname, [[dev systemName] UTF8String]);
		snprintf(Darwin_osVersion, sizeof(Darwin_osVersion), "%s (%s)", Darwin_uname.release, [[dev systemVersion] UTF8String]);
		
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
