/* Miwa Portable Runtime
 *
 * sys_win32.c
 * Author: Alexandru Naiman
 *
 * Win32 Platform Support
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (c) 2018-2019, Alexandru Naiman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define WIN32_LEAN_AND_MEAN
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <winsock2.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE	3
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <system/defs.h>
#include <system/system.h>
#include <system/compat.h>

#include "compat_win32.h"

#ifndef PROCESSOR_ARCHITECTURE_INTEL
	#define PROCESSOR_ARCHITECTURE_INTEL			0
#endif

#ifndef PROCESSOR_ARCHITECTURE_MIPS
	#define PROCESSOR_ARCHITECTURE_MIPS				1
#endif

#ifndef PROCESSOR_ARCHITECTURE_ALPHA
	#define PROCESSOR_ARCHITECTURE_ALPHA			2
#endif

#ifndef PROCESSOR_ARCHITECTURE_PPC
	#define PROCESSOR_ARCHITECTURE_PPC				3
#endif

#ifndef PROCESSOR_ARCHITECTURE_SHX
	#define PROCESSOR_ARCHITECTURE_SHX				4
#endif

#ifndef PROCESSOR_ARCHITECTURE_ARM
	#define PROCESSOR_ARCHITECTURE_ARM				5
#endif

#ifndef PROCESSOR_ARCHITECTURE_IA64
	#define PROCESSOR_ARCHITECTURE_IA64				6
#endif

#ifndef PROCESSOR_ARCHITECTURE_ALPHA64
	#define PROCESSOR_ARCHITECTURE_ALPHA64			7
#endif

#ifndef PROCESSOR_ARCHITECTURE_MSIL
	#define PROCESSOR_ARCHITECTURE_MSIL				8
#endif

#ifndef PROCESSOR_ARCHITECTURE_AMD64
	#define PROCESSOR_ARCHITECTURE_AMD64			9
#endif

#ifndef PROCESSOR_ARCHITECTURE_IA32_ON_WIN64
	#define PROCESSOR_ARCHITECTURE_IA32_ON_WIN64	10
#endif

#ifndef PROCESSOR_ARCHITECTURE_NEUTRAL
	#define PROCESSOR_ARCHITECTURE_NEUTRAL			11
#endif

#ifndef PROCESSOR_ARCHITECTURE_ARM64
	#define PROCESSOR_ARCHITECTURE_ARM64			12
#endif

#ifndef PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64
	#define PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64	13
#endif

#if _MSC_VER < 1400
	#define HMODULE HINSTANCE
	#define _Out_ OUT
#endif

#define INFO_STR_LEN	512

static HMODULE _win32_kernel32;
static HMODULE _win32_ws2_32;

static char _win32_name[INFO_STR_LEN] = { 0x0 };
static char _win32_release[INFO_STR_LEN] = { 0x0 };
static char _win32_version[INFO_STR_LEN] = { 0x0 };
static char _win32_machine[INFO_STR_LEN] = { 0x0 };
static char _win32_hostname[INFO_STR_LEN] = { 0x0 };
static char _win32_cpu_name[INFO_STR_LEN] = { 0x0 };
static uint32_t _win32_num_cpu = 0;
static uint32_t _win32_cpu_freq = 0;
static char _win32_log_dir[MAX_PATH] = { 0x0 };
static char _win32_temp_dir[MAX_PATH] = { 0x0 };
static char _win32_cache_dir[MAX_PATH] = { 0x0 };
static char _win32_runtime_dir[MAX_PATH] = { 0x0 };
static char _win32_tmp_buff[MAX_PATH] = { 0x0 };

extern char _miwa_sys_name[SYS_MAX_NAME];

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	#define VER_SUITE_EMBEDDEDNT			0x00000040
	#define VER_NT_WORKSTATION				0x00000001

	#define PROCESSOR_ARCHITECTURE_AMD64	9

	#define wSuiteMask wReserved[0]
	#define wProductType wReserved[1]

	#define ZeroMemroy(a, b) memset(a, 0x0, b)
#else
	#include <intrin.h>
#endif

int
platform_init(void)
{
	int cpu_info[4] = { -1 };
	HKEY key;
	WSADATA wsa_data;
	DWORD size = 0;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
		0,
		KEY_READ,
		&key) == ERROR_SUCCESS) {

		size = sizeof(_win32_cpu_name);
		RegQueryValueEx(key, "ProcessorNameString", 0, 0, _win32_cpu_name, &size);

		sizeof(_win32_cpu_freq);
		RegQueryValueEx(key, "~MHz", 0, 0, (unsigned char *)&_win32_cpu_freq, &size);

		RegCloseKey(key);
	} else {
	#if	_MSC_VER > 1200
		// cpuid fallback; only works on x86
		__cpuid(cpu_info, 0x80000002);
		memcpy(_win32_cpu_name, cpu_info, sizeof(cpu_info));

		__cpuid(cpu_info, 0x80000003);
		memcpy(_win32_cpu_name + 16, cpu_info, sizeof(cpu_info));

		__cpuid(cpu_info, 0x80000004);
		memcpy(_win32_cpu_name + 32, cpu_info, sizeof(cpu_info));
	#else
		snprintf(_win32_cpu_name, 512, "Unknown");
	#endif		
	}

	_win32_kernel32 = LoadLibrary("kernel32.dll");
	_win32_ws2_32 = LoadLibrary("ws2_32.dll");

	win32_GetNativeSystemInfo = GetProcAddress(_win32_kernel32,
						"GetNativeSystemInfo");

	win32_GlobalMemoryStatusEx = GetProcAddress(_win32_kernel32,
						"GlobalMemoryStatusEx");

	win32_SwitchToThread = GetProcAddress(_win32_kernel32,
						"SwitchToThread");
	if (!win32_SwitchToThread)
		win32_SwitchToThread = miwa_SwitchToThread;

	// Critical section
	win32_TryEnterCriticalSection = GetProcAddress(_win32_kernel32,
						"TryEnterCriticalSection");
	if (!win32_TryEnterCriticalSection)
		win32_TryEnterCriticalSection = miwa_TryEnterCriticalSection;

	if (_win32_ws2_32) {
		win32_WSAStartup = GetProcAddress(_win32_ws2_32, "WSAStartup");
		win32_WSACleanup = GetProcAddress(_win32_ws2_32, "WSACleanup");

		if (win32_WSAStartup(MAKEWORD(2, 2), &wsa_data))
			return SYS_ERROR;
	}

	return SYS_OK;
}

void
platform_release(void)
{
	if (_win32_ws2_32) {
		win32_WSACleanup();

		FreeLibrary(_win32_ws2_32);
	}

	FreeLibrary(_win32_kernel32);
}

static INLINE void
_win32_get_version_info(void)
{
	bool embedded = false, server = false;
	char ver_str[256], build_str[256], sp_str[256];
	HKEY key = 0;
	DWORD size = 0;
	DWORD platform_id = 0, ver_major = 0, ver_minor = 0, ver_build;
	OSVERSIONINFOA ver;
	OSVERSIONINFOEXA ver_ex;

	ZeroMemory(&ver, sizeof(ver));
	ver.dwOSVersionInfoSize = sizeof(ver);

	ZeroMemory(&ver_ex, sizeof(ver_ex));
	ver_ex.dwOSVersionInfoSize = sizeof(ver_ex);

	ZeroMemory(sp_str, 256);

	if (GetVersionExA((OSVERSIONINFOA *)&ver_ex)) {
		if (ver_ex.wSuiteMask & VER_SUITE_EMBEDDEDNT)
			embedded = true;

		if (ver_ex.wProductType != VER_NT_WORKSTATION)
			server = true;

		platform_id = ver_ex.dwPlatformId;
		ver_major = ver_ex.dwMajorVersion;
		ver_minor = ver_ex.dwMinorVersion;
		ver_build = ver_ex.dwBuildNumber;

		if (ver_ex.wServicePackMinor)
			snprintf(sp_str, 256, "Service Pack %d.%d",
				ver_ex.wServicePackMajor,
				ver_ex.wServicePackMinor);
		else if (ver_ex.wServicePackMajor)
			snprintf(sp_str, 256, "Service Pack %d",
				ver_ex.wServicePackMajor);

	} else if (GetVersionExA((OSVERSIONINFOA *)&ver)) {
		platform_id = ver.dwPlatformId;
		ver_major = ver.dwMajorVersion;
		ver_minor = ver.dwMinorVersion;
		ver_build = ver.dwBuildNumber;

		snprintf(sp_str, 256, "%s", ver.szCSDVersion);
	}

	if (platform_id == VER_PLATFORM_WIN32s)
		snprintf(_win32_name, INFO_STR_LEN, "%s", "Win32s");
	else if (platform_id == VER_PLATFORM_WIN32_WINDOWS)
		snprintf(_win32_name, INFO_STR_LEN, "%s", "Windows");
	else if (platform_id == VER_PLATFORM_WIN32_NT)
		snprintf(_win32_name, INFO_STR_LEN, "%s%s%s",
			"Windows NT", embedded ? " Embedded" : "",
			server ? " Server" : "");
	else if (ver.dwPlatformId == VER_PLATFORM_WIN32_CE)
		snprintf(_win32_name, INFO_STR_LEN, "%s", "Windows CE");

	if (ver_major == 6 && ver_minor == 2 && ver_build == 9200) {
		/*
		 * Windows 8.1 or 10
		 * In Windows 10 the real version number
		 * is hidden by GetVersionExA
		 * so we have to look in the registry
		 * for it
		 */

		RegOpenKeyExA(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
			0, KEY_READ, &key);

		size = sizeof(ver_str);
		RegQueryValueExA(key, "CurrentVersion", 0, NULL,
			ver_str, &size);
		size = sizeof(build_str);
		RegQueryValueExA(key, "CurrentBuild", 0, NULL,
			build_str, &size);

		snprintf(_win32_version, 100, "%s.%s", ver_str, build_str);
	} else {
		snprintf(_win32_version, 100, "%d.%d.%d",
			ver_major, ver_minor, ver_build);
	}

	if (sp_str[0] != 0x0) {
		size = strlen(_win32_version);
		snprintf(_win32_version + size, 100 - size, " %s", sp_str);
	}
}

static INLINE void
_win32_get_sys_info(void)
{
	SYSTEM_INFO info;
	
	ZeroMemory(&info, sizeof(info));

	if (win32_GetNativeSystemInfo)
		win32_GetNativeSystemInfo(&info);
	else
		GetSystemInfo(&info);

	switch (info.wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_AMD64: snprintf(_win32_machine, 512, "%s", "x64"); break;		
		case PROCESSOR_ARCHITECTURE_INTEL: snprintf(_win32_machine, 512, "%s", "x86"); break;
		case PROCESSOR_ARCHITECTURE_ARM64: snprintf(_win32_machine, 512, "%s", "arm64"); break;
		case PROCESSOR_ARCHITECTURE_ARM: snprintf(_win32_machine, 512, "%s", "arm"); break;
		case PROCESSOR_ARCHITECTURE_MIPS: snprintf(_win32_machine, 512, "%s", "MIPS"); break;
		case PROCESSOR_ARCHITECTURE_ALPHA: snprintf(_win32_machine, 512, "%s", "Alpha"); break;
		case PROCESSOR_ARCHITECTURE_ALPHA64: snprintf(_win32_machine, 512, "%s", "Alpha 64"); break;
		case PROCESSOR_ARCHITECTURE_SHX: snprintf(_win32_machine, 512, "%s", "SuperH"); break;
		case PROCESSOR_ARCHITECTURE_PPC: snprintf(_win32_machine, 512, "%s", "PowerPC"); break;
		case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64: snprintf(_win32_machine, 512, "%s", "Itanium on x64"); break;
		case PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64: snprintf(_win32_machine, 512, "%s", "arm on x64"); break;
		case PROCESSOR_ARCHITECTURE_IA64: snprintf(_win32_machine, 512, "%s", "Itanium"); break;
		case PROCESSOR_ARCHITECTURE_MSIL: snprintf(_win32_machine, 512, "%s", "msil"); break;
		
		default: snprintf(_win32_machine, 512, "%s", "Unknown"); break;
	}

	_win32_num_cpu = info.dwNumberOfProcessors;
}

static INLINE void
_win32_mem_info(uint64_t *free, uint64_t *used, uint64_t *total)
{
	MEMORYSTATUSEX mem_status_ex;
	MEMORYSTATUS mem_status;

	if (win32_GlobalMemoryStatusEx) {
		ZeroMemory(&mem_status_ex, sizeof(mem_status_ex));
		mem_status_ex.dwLength = sizeof(mem_status_ex);

		win32_GlobalMemoryStatusEx(&mem_status_ex);

		if (free)
			*free = mem_status_ex.ullAvailPhys;

		if (used)
			*used = (mem_status_ex.ullTotalPhys - mem_status_ex.ullAvailPhys);

		if (total)
			*total = mem_status_ex.ullTotalPhys;
	} else {
		ZeroMemory(&mem_status, sizeof(mem_status));
		mem_status.dwLength = sizeof(mem_status);

		GlobalMemoryStatus(&mem_status);

		if (free)
			*free = mem_status.dwAvailPhys;

		if (used)
			*used = (mem_status.dwTotalPhys - mem_status.dwAvailPhys);

		if (total)
			*total = mem_status.dwTotalPhys;
	}
}

const char *
sys_os_name(void)
{
	if (!strlen(_win32_name))
		_win32_get_version_info();

	return _win32_name;
}

const char *
sys_os_version(void)
{
	if (!strlen(_win32_version))
		_win32_get_version_info();

	return _win32_version;
}

const char *
sys_machine(void)
{
	if (!strlen(_win32_machine))
		_win32_get_sys_info();

	return _win32_machine;
}

const char *
sys_hostname(void)
{
	if (!strlen(_win32_hostname)) {
		DWORD size = 512;
		GetComputerNameA(_win32_hostname, &size);
	}

	return _win32_hostname;
}

const char *
sys_cpu_name(void)
{
	return _win32_cpu_name;
}

uint32_t
sys_cpu_freq(void)
{
	return _win32_cpu_freq;
}

uint32_t
sys_cpu_count(void)
{
	if (!_win32_num_cpu)
		_win32_get_sys_info();

	return _win32_num_cpu;
}

uint64_t
sys_mem_used(void)
{
	uint64_t mem;
	_win32_mem_info(0, &mem, 0);
	return mem;
}

uint64_t
sys_mem_free(void)
{
	uint64_t mem;
	_win32_mem_info(&mem, 0, 0);
	return mem;
}

uint64_t
sys_mem_total(void)
{
	uint64_t mem;
	_win32_mem_info(0, 0, &mem);
	return mem;
}

void
sys_sleep(uint32_t sec)
{
	Sleep(sec * 1000);
}

void
sys_msleep(uint32_t msec)
{
	Sleep(msec);
}

void
sys_usleep(uint32_t usec)
{
	Sleep(usec / 1000);
}

void *
sys_load_library(const char *library)
{
	if (!library)
		return NULL;
	else
		return LoadLibraryA(library);
}

void *
sys_get_proc_address(void *library,
	const char *proc)
{
	if (!library || !proc)
		return NULL;
	else
		return GetProcAddress(library, proc);
}

void
sys_unload_library(void *library)
{
	if (!library)
		return;

	FreeLibrary(library);
}

const char *
sys_log_dir(void)
{
	if (_win32_log_dir[0] == 0x0) {
		memset(_win32_tmp_buff, 0x0, MAX_PATH);
		memset(_win32_log_dir, 0x0, MAX_PATH);

		snprintf(_win32_tmp_buff, MAX_PATH, "%%APPDATA%%/%s/log", _miwa_sys_name);
		ExpandEnvironmentStrings(_win32_tmp_buff, _win32_log_dir, MAX_PATH);

		if (sys_directory_exists(_win32_log_dir) < 0)
			sys_create_directory(_win32_log_dir);
	}

	return _win32_log_dir;
}

const char *
sys_temp_dir(void)
{
	if (_win32_temp_dir[0] == 0x0) {
		memset(_win32_tmp_buff, 0x0, MAX_PATH);
		memset(_win32_temp_dir, 0x0, MAX_PATH);

		snprintf(_win32_tmp_buff, MAX_PATH, "%%TEMP%%/%s", _miwa_sys_name);
		ExpandEnvironmentStrings(_win32_tmp_buff, _win32_temp_dir, MAX_PATH);

		if (sys_directory_exists(_win32_temp_dir) < 0)
			sys_create_directory(_win32_temp_dir);
	}

	return _win32_temp_dir;
}

const char *
sys_cache_dir(void)
{
	if (_win32_cache_dir[0] == 0x0) {
		memset(_win32_tmp_buff, 0x0, MAX_PATH);
		memset(_win32_cache_dir, 0x0, MAX_PATH);

		snprintf(_win32_tmp_buff, MAX_PATH, "%%APPDATA%%/%s/cache", _miwa_sys_name);
		ExpandEnvironmentStrings(_win32_tmp_buff, _win32_cache_dir, MAX_PATH);

		if (sys_directory_exists(_win32_cache_dir) < 0)
			sys_create_directory(_win32_cache_dir);
	}

	return _win32_cache_dir;
}

const char *
sys_runtime_dir(void)
{
	if (_win32_runtime_dir[0] == 0x0) {
		memset(_win32_tmp_buff, 0x0, MAX_PATH);
		memset(_win32_runtime_dir, 0x0, MAX_PATH);

		snprintf(_win32_tmp_buff, MAX_PATH, "%%APPDATA%%/%s/run", _miwa_sys_name);
		ExpandEnvironmentStrings(_win32_tmp_buff, _win32_runtime_dir, MAX_PATH);

		if (sys_directory_exists(_win32_runtime_dir) < 0)
			sys_create_directory(_win32_runtime_dir);
	}

	return _win32_runtime_dir;
}

int
sys_file_exists(const char *file)
{
	struct stat st;

	if (_access(file, 0) != 0)
		return -1;

	stat(file, &st);

	return ((st.st_mode & S_IFDIR) == 0) ? 0 : -2;
}

int
sys_directory_exists(const char *dir)
{
	struct stat st;

	if (_access(dir, 0) != 0)
		return -1;

	stat(dir, &st);

	return ((st.st_mode & S_IFDIR) != 0) ? 0 : -2;
}

int
sys_create_directory(const char *path)
{
	char dir[MAX_PATH];
	const char *end = 0, *tmp = 0;
	
	ZeroMemory(dir, MAX_PATH);

	end = strchr(path, '\\');
	if (!end)
		end = strchr(path, '/');

	while (end) {
		ZeroMemory(dir, MAX_PATH);
		strncpy(dir, path, end - path + 1);

		(void)CreateDirectory(dir, 0);

		tmp = end;
		end = strchr(++end, '\\');
		if (!end)
			end = strchr(++tmp, '/');
	}

	if (!CreateDirectory(path, 0))
		if (GetLastError() != ERROR_ALREADY_EXISTS)
			return -1;

	return 0;
}

