/* Miwa Portable Runtime
 *
 * sys_unix.c
 * Author: Alexandru Naiman
 *
 * UNIX Platform Support
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

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>

#if defined(__FreeBSD__) || defined(__OpenBSD__) || \
	defined(__MidnightBSD__) || defined(__DragonFly__) || \
	defined(__NetBSD__) || (defined(__APPLE__) && defined(__MACH__))
	#include <sys/sysctl.h>
#endif

#ifdef _IOS
#include <CoreFoundation/CoreFoundation.h>

static char _nix_tmp_buff[PATH_MAX] = { 0x0 };
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <system/log.h>
#include <system/defs.h>
#include <system/system.h>
#include <system/compat.h>
#include <system/platform.h>

#define PLATFORM_STR_SIZE	512

static char _nix_name[PLATFORM_STR_SIZE] = { 0x0 };
static char _nix_release[PLATFORM_STR_SIZE] = { 0x0 };
static char _nix_version[PLATFORM_STR_SIZE] = { 0x0 };
static char _nix_machine[PLATFORM_STR_SIZE] = { 0x0 };
static char _nix_hostname[PLATFORM_STR_SIZE] = { 0x0 };
static char _nix_cpu_name[PLATFORM_STR_SIZE] = { 0x0 };
static uint32_t _nix_cpu_freq = 0;
static char _nix_log_dir[PATH_MAX] = { 0x0 };
static char _nix_temp_dir[PATH_MAX] = { 0x0 };
static char _nix_cache_dir[PATH_MAX] = { 0x0 };
static char _nix_runtime_dir[PATH_MAX] = { 0x0 };

extern char _miwa_sys_name[SYS_MAX_NAME];

#define NIX_PLATFORM_MODULE		"UNIX_Platform"

#ifdef SYS_PLATFORM_APPLE
#include <mach/mach_time.h>

extern uint64_t __miwa_mach_timer_freq;
#endif

#ifdef __ANDROID__
void _thread_exit_handler(int sig)
{
	pthread_exit(0);
}
#endif

int
platform_init(void)
{
#ifdef __ANDROID__
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = _thread_exit_handler;
	if (sigaction(SIGUSR1, &act, 0) != 0)
		return SYS_ERROR;
#endif
	
#ifdef SYS_PLATFORM_APPLE
	mach_timebase_info_data_t tb;
    mach_timebase_info(&tb);
	
    __miwa_mach_timer_freq = tb.numer / tb.denom;
#endif

#if defined(__linux__)
	char buff[512];
	FILE *fp = fopen("/proc/cpuinfo", "r");

	_nix_cpu_freq = 0.0;
	memset(buff, 0x0, 512);

	if (!fp) {
		snprintf(_nix_cpu_name, PLATFORM_STR_SIZE, "Unknown");
	} else {
		while (fgets(buff, 512, fp)) {
			if (strstr(buff, "model name")) {
				char *ptr = strchr(buff, ':');
				snprintf(_nix_cpu_name, PLATFORM_STR_SIZE, "%s", ptr ? ptr + 2 : "Unknown");
			} else if (strstr(buff, "cpu MHz")) {
				char *ptr = strchr(buff, ':');
				if (ptr) {
					ptr += 2;
					ptr[strlen(ptr) - 1] = 0x0;
					_nix_cpu_freq = atoi(ptr);
				}
				break;
			}
			memset(buff, 0x0, 512);
		}
		fclose(fp);
	}
#elif defined(sun) || defined(__sun)
	char buff[512], last_buff[512];
	FILE *fp = popen("/usr/sbin/psrinfo -pv", "r");

	while (fgets(buff, sizeof(buff), fp) != NULL) {
		char *p = strstr(buff, "clock");
		if (p) {
			p = strchr(++p, ' ');
			++p;
			 p[strlen(p) - 6] = '\0';
			_nix_cpu_freq = atoi(p);
		} else if (buff[0] == '\t') {
			char *str = &buff[1];
			buff[strlen(str)] = '\0';
			snprintf(_nix_cpu_name, PLATFORM_STR_SIZE, "%s", str);
		}
		memcpy(last_buff, buff, 512);
		printf("%s\n", last_buff);
	}

	if (!strlen(_nix_cpu_name))
		snprintf(_nix_cpu_name, PLATFORM_STR_SIZE, "%s", last_buff);

	pclose(fp);
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || \
	defined(__MidnightBSD__) || defined(__DragonFly__) || \
	defined(__NetBSD__)
	char buff[512];

	FILE *fp = popen("/sbin/sysctl -n hw.model", "r");
	if (fgets(buff, sizeof(buff), fp) != NULL) {
		buff[strlen(buff) - 1] = 0x0;
		snprintf(_nix_cpu_name, PLATFORM_STR_SIZE, "%s", buff);
	} else {
		snprintf(_nix_cpu_name, PLATFORM_STR_SIZE, "Unknown");
	}
	pclose(fp);

	fp = popen("/sbin/sysctl -n hw.clockrate", "r");
	if (fgets(buff, sizeof(buff), fp) != NULL) {
		buff[strlen(buff) - 1] = 0x0;
		_nix_cpu_freq = atoi(buff);
	}
	pclose(fp);
#elif defined(__APPLE__) && defined(__MACH__)
	FILE *fp = NULL;
	char buff[512];

	_nix_cpu_freq = 0;
	
	fp = popen("/usr/sbin/sysctl -n machdep.cpu.brand_string", "r");
	if (fgets(buff, sizeof(buff), fp) != NULL) {
		buff[strlen(buff) - 1] = 0x0;
		snprintf(_nix_cpu_name, PLATFORM_STR_SIZE, "%s", buff);
               
		char *c = strchr(buff, '@') + 2;
		float f = atof(c) * 1000.f;
		_nix_cpu_freq = f;
	} else { 
		pclose(fp);
		
		fp = popen("system_profiler | grep 'Processor Name' | cut -c 23-", "r");
		if (fgets(buff, sizeof(buff), fp) != NULL) {
			buff[strlen(buff) - 1] = 0x0;
			snprintf(_nix_cpu_name, PLATFORM_STR_SIZE, "%s", buff);
		} else {
			pclose(fp);
			fp = popen("system_profiler | grep 'CPU Type' | cut -c 17-", "r");
			if (fgets(buff, sizeof(buff), fp) != NULL) {
				buff[strlen(buff) - 1] = 0x0;
				snprintf(_nix_cpu_name, PLATFORM_STR_SIZE, "%s", buff);
			} else {
				snprintf(_nix_cpu_name, PLATFORM_STR_SIZE, "Unknown");
			}
		}
	}
	pclose(fp);
	
	//sysctlbyname("hw.cpufrequency_max", 1, &freq, sizeof(freq), 0, 0);
#else
	snprintf(_nix_cpu_name, PLATFORM_STR_SIZE, "Unknown");
#endif

	if (_nix_cpu_name[strlen(_nix_cpu_name) - 1] == '\n')
		_nix_cpu_name[strlen(_nix_cpu_name) - 1] = 0x0;

	return SYS_OK;
}

void
platform_release(void)
{
}

static inline void
_nix_uname(void)
{
	struct utsname name;

	uname(&name);

#ifdef SYS_PLATFORM_APPLE
	#ifdef SYS_PLATFORM_IOS
		snprintf(_nix_name, PLATFORM_STR_SIZE, "%s (iOS)", name.sysname);
	#else
		snprintf(_nix_name, PLATFORM_STR_SIZE, "%s (Mac OS X)", name.sysname);
	#endif
#else
	strlcpy(_nix_name, name.sysname, PLATFORM_STR_SIZE);
#endif
	
	strlcpy(_nix_release, name.release, PLATFORM_STR_SIZE);
	strlcpy(_nix_version, name.version, PLATFORM_STR_SIZE);
	strlcpy(_nix_machine, name.machine, PLATFORM_STR_SIZE);
	strlcpy(_nix_hostname, name.nodename, PLATFORM_STR_SIZE);
}

const char *
sys_os_name(void)
{
	if (!strlen(_nix_name))
		_nix_uname();

	return _nix_name;
}

const char *
sys_os_version(void)
{
	if (!strlen(_nix_release))
		_nix_uname();

	return _nix_release;
}

const char *
sys_machine(void)
{
	if (!strlen(_nix_machine))
		_nix_uname();

	return _nix_machine;
}

const char *
sys_hostname(void)
{
	if (!strlen(_nix_hostname))
		_nix_uname();

	return _nix_hostname;
}

const char *
sys_cpu_name(void)
{
	return _nix_cpu_name;
}

uint32_t
sys_cpu_freq(void)
{
	return _nix_cpu_freq;
}

uint32_t
sys_cpu_count(void)
{
#ifndef _SC_NPROCESSORS_ONLN
	return sysconf(HW_NCPU);
#else
	return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

uint64_t
sys_mem_used(void)
{
	return sys_mem_total() - sys_mem_free();
}

uint64_t
sys_mem_free(void)
{
#if defined(_SC_AVPHYS_PAGES)
	uint64_t pages = sysconf(_SC_AVPHYS_PAGES);
	uint64_t page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
#elif defined(HW_PHYSMEM)
	uint64_t mem = 0;
	size_t len = sizeof(mem);
	int mib[2] = { CTL_HW, HW_USERMEM };
      
	if (sysctl(mib, 2, &mem, &len, NULL, 0) < 0)
		return 0;

	return mem;
#else
	#error FATAL: sys_mem_free NOT IMPLEMENTED FOR THIS PLATFORM
	return 0;
#endif
}

uint64_t
sys_mem_total(void)
{
#if defined(_SC_PHYS_PAGES)
	uint64_t pages = sysconf(_SC_PHYS_PAGES);
	uint64_t page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
#elif defined(HW_PHYSMEM)
	uint64_t mem = 0;
	size_t len = sizeof(mem);
#ifdef SYS_PLATFORM_APPLE
	static int mib[2] = { CTL_HW, HW_MEMSIZE };
#else
	static int mib[2] = { CTL_HW, HW_PHYSMEM };
#endif
       
	if (sysctl(mib, 2, &mem, &len, NULL, 0) < 0)
		return 0;

	return mem;
#else
	#error FATAL: sys_mem_total NOT IMPLEMENTED FOR THIS PLATFORM
	return 0;
#endif
}

void
sys_sleep(uint32_t sec)
{
	sleep(sec);
}

void
sys_msleep(uint32_t msec)
{
	usleep(msec * 1000);
}

void
sys_usleep(uint32_t usec)
{
	usleep(usec);
}

void *
sys_load_library(const char *library)
{
	char path[2048];
	char *prefix = "", *suffix = "";
	size_t len = 0;
	void *ret = NULL;

	if (!library)
		return NULL;

	if (access(library, R_OK) < 0) {
		if (strncmp(library, "lib", 3))
			prefix = "lib";

		len = strlen(library);

#ifdef __APPLE__
		if (len > 7) {
			if (strncmp(library + len - 6, ".dylib", 6))
				suffix = ".dylib";
		} else {
			suffix = ".dylib";
		}
#else
		if (len > 4) {
			if (strncmp(library + len - 3, ".so", 3))
				suffix = ".so";
		} else {
			suffix = ".so";
		}
#endif
	}

	snprintf(path, 2048, "%s%s%s", prefix, library, suffix);

	ret = dlopen(path, RTLD_NOW);
	if (ret)
		return ret;

	log_entry(NIX_PLATFORM_MODULE, LOG_CRITICAL,
		"dlopen() for [%s] failed with [%s]", path, dlerror());

	return NULL;
}

void *
sys_get_proc_address(void *library,
	const char *proc)
{
	if (!library || !proc)
		return NULL;
	else
		return dlsym(library, proc);
}

void
sys_unload_library(void *library)
{
	if (library)
		dlclose(library);
}

const char *
sys_log_dir(void)
{
	if (_nix_log_dir[0] == 0x0) {
	#if defined(_IOS)
		CFURLRef url = CFCopyHomeDirectoryURL();
		CFStringRef path = CFURLCopyPath(url);
		
		memset(_nix_tmp_buff, 0x0, PATH_MAX);
		CFStringGetCString(path, _nix_tmp_buff, PATH_MAX, kCFStringEncodingUTF8);
		
		CFRelease(path);
		CFRelease(url);
		
		snprintf(_nix_cache_dir, PATH_MAX, "%s/Library/Logs", _nix_tmp_buff);
	#elif defined(__ANDROID__)
	#else
		snprintf(_nix_log_dir, PATH_MAX, "/var/log/%s", _miwa_sys_name);
	#endif

		if (sys_directory_exists(_nix_log_dir) < 0)
			sys_create_directory(_nix_log_dir);
	}

	return _nix_log_dir;
}

const char *
sys_temp_dir(void)
{
	if (_nix_temp_dir[0] == 0x0) {
	#if defined(_IOS)
		CFURLRef url = CFCopyHomeDirectoryURL();
		CFStringRef path = CFURLCopyPath(url);
		
		memset(_nix_tmp_buff, 0x0, PATH_MAX);
		CFStringGetCString(path, _nix_tmp_buff, PATH_MAX, kCFStringEncodingUTF8);
		
		CFRelease(path);
		CFRelease(url);
		
		snprintf(_nix_cache_dir, PATH_MAX, "%s/tmp", _nix_tmp_buff);
	#elif defined(__ANDROID__)
	#else
		snprintf(_nix_temp_dir, PATH_MAX, "/tmp/%s", _miwa_sys_name);
	#endif

		if (sys_directory_exists(_nix_temp_dir) < 0)
			sys_create_directory(_nix_temp_dir);
	}

	return _nix_temp_dir;
}

const char *
sys_cache_dir(void)
{
	if (_nix_cache_dir[0] == 0x0) {
	#if defined(_IOS)
		CFURLRef url = CFCopyHomeDirectoryURL();
		CFStringRef path = CFURLCopyPath(url);
		
		memset(_nix_tmp_buff, 0x0, PATH_MAX);
		CFStringGetCString(path, _nix_tmp_buff, PATH_MAX, kCFStringEncodingUTF8);
		
		CFRelease(path);
		CFRelease(url);
		
		snprintf(_nix_cache_dir, PATH_MAX, "%s/Library/Cache", _nix_tmp_buff);
	#elif defined(__ANDROID__)
	#else
		snprintf(_nix_cache_dir, PATH_MAX, "/var/cache/%s", _miwa_sys_name);
	#endif

		if (sys_directory_exists(_nix_cache_dir) < 0)
			sys_create_directory(_nix_cache_dir);
	}

	return _nix_cache_dir;
}

const char *
sys_runtime_dir(void)
{
	if (_nix_runtime_dir[0] == 0x0) {
	#if defined(_IOS)
		CFURLRef url = CFCopyHomeDirectoryURL();
		CFStringRef path = CFURLCopyPath(url);
		
		memset(_nix_tmp_buff, 0x0, PATH_MAX);
		CFStringGetCString(path, _nix_tmp_buff, PATH_MAX, kCFStringEncodingUTF8);
		
		CFRelease(path);
		CFRelease(url);
		
		snprintf(_nix_cache_dir, PATH_MAX, "%s/run", _nix_tmp_buff);
	#elif defined(__ANDROID__)
	#else
		snprintf(_nix_runtime_dir, PATH_MAX, "/var/run/%s", _miwa_sys_name);
	#endif
	
		if (sys_directory_exists(_nix_runtime_dir) < 0)
			sys_create_directory(_nix_runtime_dir);
	}

	return _nix_runtime_dir;
}

int
sys_file_exists(const char *file)
{
	struct stat st;

	if (access(file, 0) != 0)
		return -1;

	stat(file, &st);

	return ((st.st_mode & S_IFDIR) == 0) ? 0 : -2;
}

int
sys_directory_exists(const char *dir)
{
	struct stat st;

	if (access(dir, 0) != 0)
		return -1;

	stat(dir, &st);

	return ((st.st_mode & S_IFDIR) != 0) ? 0 : -2;
}

int
sys_create_directory(const char *path)
{
	return mkdir(path, 0755);
}

