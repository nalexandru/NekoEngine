#ifndef NE_SYSTEM_SYSTEM_H
#define NE_SYSTEM_SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <Engine/Types.h>
#include <System/PlatformDetect.h>

#ifdef __cplusplus
extern "C" {
#endif

enum NeMsgBoxIcon
{
	MSG_ICON_NONE = 0,
	MSG_ICON_INFO = 1,
	MSG_ICON_WARN = 2,
	MSG_ICON_ERROR = 3
};

enum NeMachineType
{
	MT_PC,
	MT_XBOX_ONE,
	MT_XBOX,
	MT_SWITCH,
	MT_WII,
	MT_MOBILE,
	MT_PS4,
	MT_PS3
};

enum NeSystemCapabilityFlags
{
	SC_MMIO = 0x01
};

enum NeSystemDirectory
{
	SD_SAVE_GAME,
	SD_APP_DATA,
	SD_TEMP
};

struct NeSysVersion
{
	uint32_t major;
	uint32_t minor;
	uint32_t revision;
};

#if defined(__GNUC__) || defined(__clang__)
#	define likely(x)		__builtin_expect(!!(x), 1)
#	define unlikely(x)		__builtin_expect(!!(x), 0)
#else
#	define likely(x)		x
#	define unlikely(x)		x
#endif

int Sys_Main(int argc, char *argv[]);

bool Sys_Init(void);
void Sys_Term(void);

bool Sys_InitDbgOut(void);
void Sys_DbgOut(int color, const char *module, const char *severity, const char *text);
void Sys_TermDbgOut(void);

uint64_t Sys_Time(void);

bool Sys_MapFile(const char *path, bool write, void **ptr, uint64_t *size);
void Sys_UnmapFile(const void *ptr, uint64_t size);

const char *Sys_Hostname(void);
const char *Sys_Machine(void);
const char *Sys_CpuName(void);
uint32_t Sys_CpuFreq(void);
uint32_t Sys_CpuCount(void);
uint32_t Sys_CpuThreadCount(void);

uint64_t Sys_TotalMemory(void);
uint64_t Sys_FreeMemory(void);

const char *Sys_OperatingSystem(void);
const char *Sys_OperatingSystemVersionString(void);
struct NeSysVersion Sys_OperatingSystemVersion(void);

enum NeMachineType Sys_MachineType(void);
uint32_t Sys_Capabilities(void);

bool Sys_ScreenVisible(void);
void Sys_MessageBox(const char *title, const char *message, int icon);
bool Sys_ProcessEvents(void);

void *Sys_LoadLibrary(const char *path);
void *Sys_GetProcAddress(void *lib, const char *name);
void Sys_UnloadLibrary(void *lib);

void Sys_Sleep(uint32_t sec);
void Sys_MSleep(uint32_t msec);
void Sys_USleep(uint32_t usec);

void Sys_DirectoryPath(enum NeSystemDirectory sd, char *out, size_t len);
bool Sys_FileExists(const char *path);
bool Sys_DirectoryExists(const char *path);
bool Sys_CreateDirectory(const char *path);
void *Sys_CreateDirWatch(const char *path, enum NeFSEvent mask, NeDirWatchCallback callback, void *ud);
void Sys_DestroyDirWatch(void *handle);

void Sys_ExecutableLocation(char *buff, size_t len);

void Sys_GetWorkingDirectory(char *buff, size_t size);
void Sys_SetWorkingDirectory(const char *dir);

void Sys_UserName(char *buff, size_t len);

intptr_t Sys_GetCurrentProcess(void);
int32_t Sys_GetCurrentProcessId(void);
void Sys_WaitForProcessExit(intptr_t handle);
intptr_t Sys_Execute(char * const *argv, const char *wd, FILE **in, FILE **out, FILE **err, bool showWindow);
bool Sys_TerminateProcess(intptr_t handle);

void Sys_DebugBreak(void);

// Compatibility shivs
void *reallocarray(void *ptr, size_t nmemb, size_t size);

#if defined(SYS_PLATFORM_WINDOWS)

#define NE_GETOPT_COMPAT
#define NE_REALLOCARRAY_COMPAT

#include <direct.h>

#define aligned_alloc(alignment, size)		_aligned_malloc(size, alignment)
#define aligned_free(mem)					_aligned_free(mem)

int getopt(int nargc, char *const nargv[], const char *ostr);

extern int opterr, optind, optopt, optreset;
extern char *optarg;

size_t strlcat(char *dst, const char *src, size_t dsize);
size_t strlcpy(char *dst, const char *src, size_t dsize);

#ifndef SYS_PLATFORM_MINGW
int strcasecmp(const char *s1, const char *s2);
#endif

#elif defined(SYS_PLATFORM_UNIX) || defined(SYS_PLATFORM_NX)
#	include <unistd.h>

#define aligned_free(mem)	free(mem)
#endif

#if defined(SYS_PLATFORM_MAC)

#if !defined(MAC_OS_X_VERSION_10_7) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_7
#define NE_STRNLEN_COMPAT
size_t strnlen(const char *str, size_t maxlen);
#endif

#if !defined(MAC_OS_X_VERSION_10_15) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_15
#include <assert.h>
#include <stdlib.h>
static inline void *
aligned_alloc(size_t alignment, size_t size)
{
	void *mem = NULL;
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
	if (posix_memalign(&mem, alignment, size))
		return NULL;
#else
	assert(alignment <= 16);
	mem = malloc(size);		// Mac OS X is 16-byte aligned
#endif
	return mem;
}
#endif

#endif

#if defined(SYS_PLATFORM_LINUX)
#	include <bsd/stdlib.h>
#	include <bsd/string.h>
#endif

// Entry point macros
#if defined(SYS_PLATFORM_WINDOWS)
#define NE_MAIN																								\
	int __stdcall																							\
	WinMain(void *hInst, void *hPrevInst, char *lpCmdLine, int nCmdShow)									\
	{																										\
		return Sys_Main(__argc, __argv);																	\
	}
#else
#define NE_MAIN																								\
	int																										\
	main(int argc, char *argv[])																			\
	{																										\
		return Sys_Main(argc, argv);																		\
	}
#endif

#ifdef __cplusplus
}
#endif

#endif /* NE_SYSTEM_SYSTEM_H */

/* NekoEngine
 *
 * System.h
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
