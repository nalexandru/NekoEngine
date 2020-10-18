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
#include <mach/mach_time.h>

#define Handle __EngineHandle

#include <Input/Input.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>

#include "MacXPlatform.h"

#undef Handle
#import <Cocoa/Cocoa.h>

#include "EngineAppDelegate.h"

#define CPU_NAME_LEN	128

static uint64_t _machTimerFreq;
static int _numCpus = 0;
static struct utsname _uname;
static char _cpuName[CPU_NAME_LEN];

static inline void _CpuInfo(void);

bool
Sys_InitDbgOut(void)
{
	return true;
}

void
Sys_DbgOut(int color, const wchar_t *module, const wchar_t *severity, const wchar_t *text)
{
	char *m, *s, *t;
	
	m = Sys_Alloc(wcslen(module), sizeof(*m) + 1, MH_Transient);
	s = Sys_Alloc(wcslen(severity), sizeof(*s) + 1, MH_Transient);
	t = Sys_Alloc(wcslen(text), sizeof(*t) + 1, MH_Transient);
	
	wcstombs(m, module, wcslen(module));
	wcstombs(s, severity, wcslen(severity));
	wcstombs(t, text, wcslen(text));
	
	NSLog(@"[%s][%s]: %s\n", m, s, t);
}

void
Sys_TermDbgOut(void)
{
}

uint64_t
Sys_Time(void)
{
	return mach_absolute_time() * _machTimerFreq;
}

bool
Sys_MapFile(const char *path, bool write, void **ptr, uint64_t *size)
{
	FILE *fp = NULL;
	int fd, p = 0, f = 0, mode = 0;

	fp = fopen(path, "r");
	if (!fp)
		return false;

	fseek(fp, 0, SEEK_END);
	*size = ftell(fp);
	fclose(fp);

	if (write) {
		p = PROT_READ | PROT_WRITE;
		mode = O_RDWR;
		f = MAP_PRIVATE;
	} else {
		p = PROT_READ;
		mode = O_RDONLY;
		f = MAP_SHARED;
	}

	fd = open(path, mode);
	if (!fd)
		return false;

	*ptr = mmap(0, *size, p, f, fd, 0);

	close(fd);

	return true;
}

void
Sys_UnmapFile(const void *ptr, uint64_t size)
{
	munmap((void *)ptr, size);
}

uint32_t
Sys_TlsAlloc(void)
{
	pthread_key_t key;
	pthread_key_create(&key, NULL);
	return (uint32_t)key;
}

void *
Sys_TlsGet(uint32_t key)
{
	return pthread_getspecific((pthread_key_t)key);
}

void
Sys_TlsSet(uint32_t key, void *data)
{
	pthread_setspecific((pthread_key_t)key, data);
}

void
Sys_TlsFree(uint32_t key)
{
	pthread_key_delete((pthread_key_t)key);
}

void
Sys_Yield(void)
{
	sched_yield();
}

const char *
Sys_Hostname(void)
{
	if (!_uname.sysname[0])
		uname(&_uname);

	return _uname.nodename;
}

const char *
Sys_Machine(void)
{
	if (!_uname.sysname[0])
		uname(&_uname);

	return _uname.machine;
}

const char *
Sys_CpuName(void)
{
	return _cpuName;
}

int
Sys_NumCpus(void)
{
	return _numCpus;
}

enum MachineType
Sys_MachineType(void)
{
	return MT_PC;
}

uint32_t
Sys_Capabilities(void)
{
	return SC_MMIO;
}

bool
Sys_ScreenVisible(void)
{
	return true;
}

bool
Sys_UniversalWindows(void)
{
	return false;
}

void
Sys_MessageBox(const wchar_t *title, const wchar_t *message, int icon)
{
	char *m = Sys_Alloc(wcslen(message), sizeof(*m) + 1, MH_Transient);
	wcstombs(m, message, wcslen(message));
	
	NSAlert *a = [[NSAlert alloc] init];
	[a addButtonWithTitle:@"OK"];
	[a setMessageText:[NSString stringWithFormat:@"%s", m]];
	
	switch (icon) {
	case MSG_ICON_NONE: break;
	case MSG_ICON_INFO: [a setAlertStyle: NSInformationalAlertStyle]; break;
	case MSG_ICON_WARN: [a setAlertStyle: NSWarningAlertStyle]; break;
	case MSG_ICON_ERROR: [a setAlertStyle: NSCriticalAlertStyle]; break;
	}
	
	[a runModal];
	[a release];
}

bool
Sys_ProcessEvents(void)
{
	while (1) {
		NSEvent *e = [NSApp nextEventMatchingMask: NSAnyEventMask
							untilDate: [NSDate distantPast]
							inMode: NSDefaultRunLoopMode
							dequeue: true];
							
		if (!e)
			break;
				
		[NSApp sendEvent: e];
	}

	return true;
}

void *
Sys_LoadLibrary(const char *name)
{
	char *path = NULL;
	
	if (!name)
		return dlopen(NULL, RTLD_NOW);
	
	if (access(name, R_OK) < 0) {
		char *prefix = "", *suffix = "";
		size_t len = strlen(name);
		
		path = Sys_Alloc(sizeof(char), 2048, MH_Transient);
		if (!path)
			return NULL;
		
		if (!strchr(name, '/') && strncmp(name, "lib", 3))
			prefix = "lib";
	
		if (len < 7 || strncmp(name + len - 6, ".dylib", 6))
			suffix = ".dylib";
		
		snprintf(path, 2048, "%s%s%s", prefix, name, suffix);
	} else {
		path = (char *)name;
	}
	
	return dlopen(path, RTLD_NOW);
}

void *
Sys_GetProcAddress(void *lib, const char *name)
{
	if (!lib || !name)
		return NULL;
	return dlsym(lib, name);
}

void
Sys_UnloadLibrary(void *lib)
{
	if (lib)
		dlclose(lib);
}

bool
Sys_InitPlatform(void)
{
#ifndef _SC_NPROCESSORS_ONLN
	_numCpus = sysconf(HW_NCPU);
#else
	_numCpus = sysconf(_SC_NPROCESSORS_ONLN);
#endif

	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);
	
	_machTimerFreq = tb.numer / tb.denom;
	
	NSApplicationLoad();
	[NSApplication sharedApplication];
	[NSApp finishLaunching];
	
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
#endif
	
	EngineAppDelegate *d = [[EngineAppDelegate alloc] init];
	[NSApp setDelegate: d];
	[[NSApplication sharedApplication] setDelegate: d];

	char dataDir[2048];
	getcwd(dataDir, 2048);
	
	char *p = strrchr(dataDir, '/');
	*p = 0x0;
	
	strncat(dataDir, "/Data", 5);
	
	E_SetCVarStr(L"Engine_DataDir", dataDir);
	
	_CpuInfo();
	
	return true;
}

void
Sys_TermPlatform(void)
{
}

static inline void
_CpuInfo(void)
{
	int null;
	FILE *fp = NULL;
	char buff[512];
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
	
	fp = popen("/usr/sbin/sysctl -n machdep.cpu.brand_string", "r");
	if (fgets(buff, sizeof(buff), fp) != NULL) { // Mac OS X 10.6+
		buff[strlen(buff) - 1] = 0x0;
		snprintf(_cpuName, sizeof(_cpuName), "%s", buff);
	} else {
		pclose(fp);
		fp = popen("system_profiler SPHardwareDataType | grep 'Processor Name' | cut -c 23-", "r");
		if (fgets(buff, sizeof(buff), fp) != NULL) { // Mac OS X 10.5
			buff[strlen(buff) - 1] = 0x0;
			snprintf(_cpuName, sizeof(_cpuName), "%s", buff);
		} else { // Mac OS X 10.4, perhaps earlier too but they're not supported. (yet)
			pclose(fp);
			fp = popen("system_profiler | grep 'CPU Type' | cut -c 17-", "r");
			if (fgets(buff, sizeof(buff), fp) != NULL) {
				buff[strlen(buff) - 1] = 0x0;
				snprintf(_cpuName, sizeof(_cpuName), "%s", buff);
				
				// The most CPUs a Mac running Tiger can have is 8 (MacPro2,1)
				// However, for some reason sysconf(HW_NCPU) returns 100 on a
				// PowerBook G4 1.67 GHz (PowerBook5,8); possibly others
				if (_numCpus > 8) {
					pclose(fp);
					fp = popen("system_profiler | grep 'Number Of CPUs' | cut -c 23-", "r");
					if (fgets(buff, sizeof(buff), fp) != NULL)
						_numCpus = atoi(buff);
				}
			} else {
				snprintf(_cpuName, sizeof(_cpuName), "Unknown");
			}
		}
	}
	pclose(fp);
	
	// restore stdout, stderr
	fflush(stdout);
	dup2(sout, STDOUT_FILENO);
	
	fflush(stderr);
	dup2(serr, STDERR_FILENO);
	
	if (_cpuName[strlen(_cpuName) - 1] == '\n')
        _cpuName[strlen(_cpuName) - 1] = 0x0;
}
