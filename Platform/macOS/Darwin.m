#include <stdio.h>
#include <stdint.h>

#include <sched.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <netinet/in.h>

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/host_info.h>
#include <mach/processor_info.h>
#include <mach-o/dyld.h>

#include <Input/Input.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Engine/Application.h>
#include <Network/Network.h>

#import <Foundation/Foundation.h>

#include "macOSPlatform.h"

#define INFO_STR_LEN	128

natural_t Darwin_numCpus = 0;
uint32_t Darwin_cpuFreq;
struct utsname Darwin_uname;
char Darwin_osName[INFO_STR_LEN];
char Darwin_osVersionString[INFO_STR_LEN];
char Darwin_cpuName[INFO_STR_LEN];
NSURL *Darwin_appSupportURL = nil;
bool Darwin_screenVisible = true;

static uint64_t f_machTimerFreq;

bool
Sys_InitDbgOut(void)
{
	return true;
}

void
Sys_DbgOut(int color, const char *module, const char *severity, const char *text)
{
	AUTORELEASE_BEGIN
		NSLog(@"[%s][%s]: %s\n", module, severity, text);
	AUTORELEASE_END
}

void
Sys_TermDbgOut(void)
{
}

uint64_t
Sys_Time(void)
{
	return mach_absolute_time() * f_machTimerFreq;
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

uint64_t
Sys_TotalMemory(void)
{
	size_t mem = 0;
	size_t len = sizeof(mem);
	static int mib[2] = { CTL_HW, HW_PHYSMEM };

	if (sysctl(mib, 2, &mem, &len, NULL, 0) < 0)
		return 0;

	return mem;
}

uint64_t
Sys_FreeMemory(void)
{
	size_t mem = 0;
	size_t len = sizeof(mem);
	int mib[2] = { CTL_HW, HW_USERMEM };

	if (sysctl(mib, 2, &mem, &len, NULL, 0) < 0)
		return 0;

	return mem;
}

const char *
Sys_OperatingSystem(void)
{
	return Darwin_osName;
}

const char *
Sys_OperatingSystemVersionString(void)
{
	return Darwin_osVersionString;
}

struct NeSysVersion
Sys_OperatingSystemVersion(void)
{
	struct NeSysVersion sv;

#if defined(MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_10
	NSOperatingSystemVersion osVer = [[NSProcessInfo processInfo] operatingSystemVersion];
	sv.major = (uint32_t)osVer.majorVersion;
	sv.minor = (uint32_t)osVer.minorVersion;
	sv.revision = (uint32_t)osVer.patchVersion;
#else
	struct utsname un;
	uname(&un);
	sscanf(un.release, "%d.%d.%d", &sv.major, &sv.minor, &sv.revision);
#endif

	return sv;
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
Sys_DirectoryPath(enum NeSystemDirectory sd, char *out, size_t len)
{
	AUTORELEASE_BEGIN

	switch (sd) {
	case SD_SAVE_GAME: {
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
		NSArray *urls = [[NSFileManager defaultManager] URLsForDirectory: NSDocumentDirectory inDomains: NSUserDomainMask];
		NSURL *path = [[urls objectAtIndex: 0] URLByAppendingPathComponent: [NSString stringWithUTF8String: App_applicationInfo.name]];

		strlcpy(out, [[path path] UTF8String], len);
#else
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, true);
		NSString *path = [[paths objectAtIndex: 0] stringByAppendingPathComponent: [NSString stringWithUTF8String: App_applicationInfo.name]];

		snprintf(out, len, "%s/Save", [path UTF8String]);
#endif
	} break;
	case SD_APP_DATA: {
		const char *path = [[Darwin_appSupportURL path] UTF8String];
		strlcpy(out, path, len);
	} break;
	case SD_TEMP: {
#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
		strlcpy(out, [[[[NSFileManager defaultManager] temporaryDirectory] path] UTF8String], len);
#else
		strlcpy(out, [NSTemporaryDirectory() UTF8String], len);
#endif
	} break;
	}

	AUTORELEASE_END
}

bool
Sys_FileExists(const char *path)
{
	AUTORELEASE_BEGIN
		BOOL isDir;
		return [[NSFileManager defaultManager] fileExistsAtPath: [NSString stringWithUTF8String: path]
													isDirectory: &isDir] && !isDir;
	AUTORELEASE_END
}

bool
Sys_DirectoryExists(const char *path)
{
	AUTORELEASE_BEGIN
		BOOL isDir;
		return [[NSFileManager defaultManager] fileExistsAtPath: [NSString stringWithUTF8String: path]
													isDirectory: &isDir] && isDir;
	AUTORELEASE_END
}

bool
Sys_CreateDirectory(const char *path)
{
#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
	AUTORELEASE_BEGIN
		return [[NSFileManager defaultManager] createDirectoryAtPath: [NSString stringWithUTF8String: path]
										 withIntermediateDirectories: true
														  attributes: nil
															   error: nil];
	AUTORELEASE_END
#else
	if (!mkdir(path, 0700))
		return true;
	
	if (errno != ENOENT)
		return false;
	
	char *dir = Sys_Alloc(sizeof(*dir), 4096, MH_Transient);
	memcpy(dir, path, strlen(path));
	
	for (char *p = dir + 1; *p; ++p) {
		if (*p != '/')
			continue;
		
		*p = 0x0;
		
		if (mkdir(dir, 0700) && errno != EEXIST)
			return false;
		
		*p = '/';
	}
	
	if (mkdir(path, 0700))
		return errno == EEXIST;
	else
		return true;
#endif
}

struct DirWatch
{
	int kq, filter, fd;
	NeDirWatchCallback cb;
	void *ud;
	struct kevent ev;
};

void *
Sys_CreateDirWatch(const char *path, enum NeFSEvent mask, NeDirWatchCallback callback, void *ud)
{
	struct DirWatch *dw = Sys_Alloc(sizeof(*dw), 1, MH_System);
	if (!dw)
		return NULL;

	//struct kevent

	/*dw->fd = open(path, )
	dw->ud = ud;
	dw->kq = kqueue();
	dw->cb = callback;
	
	int fflags = 0;
	if ((mask & FE_Create) == FE_Create)
		
	if ((mask & FE_Delete) == FE_Delete)
		fflags |= NOTE_DELETE;
	if ((mask & FE_Modify) == FE_Modify)
		fflags |= NOTE_CLOSE_WRITE;
	
	EV_SET(&dw->ev, dw->fd, EVFILT_FS, EV_ADD | EV_ENABLE | EV_ONESHOT, fflags, 0, 0);*/
	
	return dw;
}

void
Sys_DestroyDirWatch(void *handle)
{
	Sys_Free(handle);
}

void
Sys_ExecutableLocation(char *buff, size_t len)
{
	uint32_t l = (uint32_t)len;
	_NSGetExecutablePath(buff, &l);
}

void
Sys_GetWorkingDirectory(char *buff, size_t len)
{
	strlcpy(buff, [[[NSFileManager defaultManager] currentDirectoryPath] UTF8String], len);
}

void
Sys_SetWorkingDirectory(const char *dir)
{
	[[NSFileManager defaultManager] changeCurrentDirectoryPath: [NSString stringWithUTF8String: dir]];
}

void
Sys_UserName(char *buff, size_t len)
{
	strlcpy(buff, [NSUserName() UTF8String], len);
}

void
Sys_ZeroMemory(void *mem, size_t len)
{
	memset(mem, 0x0, len);
}

bool
Sys_MountPlatformResources(void)
{
	NSString *path = [[NSBundle mainBundle] resourcePath];
	return E_Mount([path UTF8String], "/");
}

void
Sys_UnmountPlatformResources(void)
{
	NSString *path = [[NSBundle mainBundle] resourcePath];
	E_Unmount([path UTF8String]);
}

bool
Sys_InitDarwinPlatform(void)
{
	static bool initialized = false;
	if (initialized)
		return true;

	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);

	f_machTimerFreq = tb.numer / tb.denom;

	char dataDir[2048];
	getcwd(dataDir, 2048);

	strlcat(dataDir, "/Data", sizeof(dataDir));

	E_SetCVarStr("Engine_DataDir", dataDir);

#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
	NSArray *urls = [[NSFileManager defaultManager] URLsForDirectory: NSApplicationSupportDirectory inDomains: NSUserDomainMask];
	Darwin_appSupportURL = [[urls objectAtIndex: 0] URLByAppendingPathComponent: [NSString stringWithUTF8String: App_applicationInfo.name]];
	[Darwin_appSupportURL retain];

	if (![[NSFileManager defaultManager] fileExistsAtPath: [Darwin_appSupportURL path]])
		[[NSFileManager defaultManager] createDirectoryAtPath: [Darwin_appSupportURL path]
								  withIntermediateDirectories: TRUE
												   attributes: nil
														error: nil];
#else
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, true);
	NSString *appSupportPath = [[paths objectAtIndex: 0] stringByAppendingPathComponent: [NSString stringWithUTF8String: App_applicationInfo.name]];
	Darwin_appSupportURL = [NSURL fileURLWithPath: appSupportPath];
	[Darwin_appSupportURL retain];

	if (![[NSFileManager defaultManager] fileExistsAtPath: appSupportPath])
		Sys_CreateDirectory([appSupportPath UTF8String]);
#endif

	NSString *logPath = [[Darwin_appSupportURL path] stringByAppendingPathComponent: @"Engine.log"];
	E_SetCVarStr("Engine_LogFile", [logPath UTF8String]);

	uname(&Darwin_uname);

	initialized = true;

	return true;
}

void
Sys_TermDarwinPlatform(void)
{
	[Darwin_appSupportURL release];
}

bool
Sys_LockMemory(void *mem, size_t size)
{
	return mlock(mem, size) == 0;
}

bool
Sys_UnlockMemory(void *mem, size_t size)
{
	return munlock(mem, size) == 0;
}

void
Sys_DebugBreak(void)
{
	raise(SIGTRAP);
}

bool Net_InitPlatform(void) { return true; }

NeSocket
Net_Socket(enum NeSocketType type, enum NeSocketProto proto)
{
	int st = SOCK_STREAM, sp = IPPROTO_TCP;

	switch (type) {
	case ST_STREAM: st = SOCK_STREAM; break;
	case ST_DGRAM: st = SOCK_DGRAM; break;
	case ST_RAW: st = SOCK_RAW; break;
	}

	switch (proto) {
	case SP_TCP: sp = IPPROTO_TCP; break;
	case SP_UDP: sp = IPPROTO_UDP; break;
	}

	return socket(AF_INET, st, sp);
}

bool
Net_Connect(NeSocket socket, const char *host, uint16_t port)
{
	struct hostent *h = gethostbyname(host);
	if (!h)
		return false;

	struct sockaddr_in addr =
	{
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr = *(struct in_addr *)h->h_addr
	};
	return connect(socket, (struct sockaddr *)&addr, sizeof(addr)) == 0;
}

bool
Net_Listen(NeSocket socket, uint16_t port, int32_t backlog)
{
	struct sockaddr_in addr =
	{
		.sin_family = AF_INET,
		.sin_addr.s_addr = INADDR_ANY,
		.sin_port = htons(port)
	};
	if (bind(socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return false;

	return listen(socket, backlog) == 0;
}

int32_t
Net_Accept(NeSocket socket)
{
	return accept(socket, NULL, 0);
}

ssize_t
Net_Send(NeSocket socket, const void *data, uint32_t count)
{
	return send(socket, data, count, 0);
}

ssize_t
Net_Recv(NeSocket socket, void *data, uint32_t count)
{
	return recv(socket, data, count, 0);
}

void
Net_Close(NeSocket socket)
{
	close(socket);
}

void Net_TermPlatform(void) { }

/* NekoEngine
 *
 * Darwin.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
