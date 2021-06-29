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

#undef Handle

#import <Foundation/Foundation.h>

#define INFO_STR_LEN	128

natural_t Darwin_numCpus = 0;
uint32_t Darwin_cpuFreq;
struct utsname Darwin_uname;
char Darwin_osName[INFO_STR_LEN];
char Darwin_osVersion[INFO_STR_LEN];
char Darwin_cpuName[INFO_STR_LEN];
NSURL *Darwin_appSupportURL = nil;
bool Darwin_screenVisible = true;

static uint64_t _machTimerFreq;

bool
Sys_InitDbgOut(void)
{
	return true;
}

void
Sys_DbgOut(int color, const wchar_t *module, const wchar_t *severity, const wchar_t *text)
{
	@autoreleasepool {
		NSLog(@"[%@][%@]: %@\n",
			  [[NSString alloc] initWithBytes: module length: wcslen(module) * sizeof(wchar_t) encoding: NSUTF32LittleEndianStringEncoding],
			  [[NSString alloc] initWithBytes: severity length: wcslen(severity) * sizeof(wchar_t) encoding: NSUTF32LittleEndianStringEncoding],
			  [[NSString alloc] initWithBytes: text length: wcslen(text) * sizeof(wchar_t) encoding: NSUTF32LittleEndianStringEncoding]
		);
	}
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

const char *
Sys_Hostname(void)
{
	return Darwin_uname.nodename;
}

const char *
Sys_Machine(void)
{
	return Darwin_uname.machine;
}

const char *
Sys_CpuName(void)
{
	return Darwin_cpuName;
}

uint32_t
Sys_CpuFreq(void)
{
	return Darwin_cpuFreq;
}

uint32_t
Sys_CpuCount(void)
{
	return Darwin_numCpus;
}

uint32_t
Sys_CpuThreadCount(void)
{
	return Darwin_numCpus;
}

const char *
Sys_OperatingSystem(void)
{
	return Darwin_osName;
}

const char *
Sys_OperatingSystemVersion(void)
{
	return Darwin_osVersion;
}

uint32_t
Sys_Capabilities(void)
{
	return SC_MMIO;
}

bool
Sys_ScreenVisible(void)
{
	return Darwin_screenVisible;
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

void
Sys_Sleep(uint32_t sec)
{
	sleep(sec);
}

void
Sys_MSleep(uint32_t msec)
{
	usleep(msec * 1000);
}

void
Sys_USleep(uint32_t usec)
{
	usleep(usec);
}

void
Sys_DirectoryPath(enum SystemDirectory sd, char *out, size_t len)
{
	@autoreleasepool {
		switch (sd) {
		case SD_SAVE_GAME: {
			NSArray<NSURL *> *urls = [[NSFileManager defaultManager] URLsForDirectory: NSDocumentDirectory
																			inDomains: NSUserDomainMask];

			NSURL *path = [[urls firstObject] URLByAppendingPathComponent: [[NSString alloc] initWithBytes: App_applicationInfo.name
																									length: wcslen(App_applicationInfo.name) * sizeof(wchar_t)
																								  encoding: NSUTF32LittleEndianStringEncoding]];

			snprintf(out, len, "%s", [[path path] UTF8String]);
		} break;
		case SD_APP_DATA: {
			const char *path = [[Darwin_appSupportURL path] UTF8String];
			snprintf(out, len, "%s", path);
		} break;
		case SD_TEMP: {
			snprintf(out, len, "%s", [[[[NSFileManager defaultManager] temporaryDirectory] path] UTF8String]);
		} break;
		}
	}
}

bool
Sys_FileExists(const char *path)
{
	@autoreleasepool {
		BOOL isDir;
		return [[NSFileManager defaultManager] fileExistsAtPath: [NSString stringWithUTF8String: path]
													isDirectory: &isDir] && !isDir;
	}
}

bool
Sys_DirectoryExists(const char *path)
{
	@autoreleasepool {
		BOOL isDir;
		return [[NSFileManager defaultManager] fileExistsAtPath: [NSString stringWithUTF8String: path]
													isDirectory: &isDir] && isDir;
	}
}

bool
Sys_CreateDirectory(const char *path)
{
	@autoreleasepool {
		return [[NSFileManager defaultManager] createDirectoryAtPath: [NSString stringWithUTF8String: path]
										 withIntermediateDirectories: true
														  attributes: nil
															   error: nil];
	}
}

void
Sys_ZeroMemory(void *mem, size_t len)
{
	memset(mem, 0x0, len);
}

bool
Sys_InitDarwinPlatform(void)
{
	static bool _initialized = false;
	
	if (_initialized)
		return true;
	
	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);
	
	_machTimerFreq = tb.numer / tb.denom;
	
	char dataDir[2048];
	getcwd(dataDir, 2048);
	
	strncat(dataDir, "/Data", 5);
	
	E_SetCVarStr(L"Engine_DataDir", dataDir);
	
	NSArray<NSURL *> *urls = [[NSFileManager defaultManager] URLsForDirectory: NSApplicationSupportDirectory
																	inDomains: NSUserDomainMask];
	
	Darwin_appSupportURL = [[urls firstObject] URLByAppendingPathComponent: [[NSString alloc] initWithBytes: App_applicationInfo.name
																									 length: wcslen(App_applicationInfo.name) * sizeof(wchar_t)
																								   encoding: NSUTF32LittleEndianStringEncoding]];
	[Darwin_appSupportURL retain];
	
	if (![[NSFileManager defaultManager] fileExistsAtPath: [Darwin_appSupportURL path]])
		[[NSFileManager defaultManager] createDirectoryAtPath: [Darwin_appSupportURL path]
								  withIntermediateDirectories: TRUE
												   attributes: nil
														error: nil];
	
	NSString *appSupportPath = [[Darwin_appSupportURL path] stringByAppendingPathComponent: @"Engine.log"];
	E_SetCVarStr(L"Engine_LogFile", [appSupportPath UTF8String]);
	
	uname(&Darwin_uname);
	
	_initialized = true;
	
	return true;
}

void
Sys_TermDarwinPlatform(void)
{
	[Darwin_appSupportURL release];
}
