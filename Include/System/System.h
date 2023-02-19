#ifndef _NE_SYSTEM_SYSTEM_H_
#define _NE_SYSTEM_SYSTEM_H_

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
struct NeSysVersion Sys_OperatingSystemVersion(void);;

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

void Sys_ExecutableLocation(char *buff, size_t len);

void Sys_GetWorkingDirectory(char *buff, size_t size);
void Sys_SetWorkingDirectory(const char *dir);

void Sys_UserName(char *buff, size_t len);

intptr_t Sys_GetCurrentProcess();
int32_t Sys_GetCurrentProcessId();
void Sys_WaitForProcessExit(intptr_t handle);
intptr_t Sys_Execute(char * const *argv, const char *wd, FILE **in, FILE **out, FILE **err, bool showWindow);
bool Sys_TerminateProcess(intptr_t handle);

// Compatibility shivs
void *reallocarray(void *ptr, size_t nmemb, size_t size);

#if defined(SYS_PLATFORM_WINDOWS)

#include <direct.h>

#define aligned_alloc(alignment, size) _aligned_malloc(size, alignment)

int getopt(int nargc, char *const nargv[], const char *ostr);

extern int opterr, optind, optopt, optreset;
extern char *optarg;

size_t strlcat(char *dst, const char *src, size_t dsize);

#elif defined(SYS_PLATFORM_UNIX) || defined(SYS_PLATFORM_NX)
#	include <unistd.h>
#endif

#if defined(SYS_PLATFORM_LINUX)
#	include <bsd/stdlib.h>
#	include <bsd/string.h>
#endif

#ifdef __cplusplus
}
#endif

#endif /* _NE_SYSTEM_SYSTEM_H_ */

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
