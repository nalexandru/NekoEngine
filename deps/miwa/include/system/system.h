/* Miwa Portable Runtime
 *
 * system.h
 * Author: Alexandru Naiman
 *
 * System
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

#ifndef _MIWA_SYSTEM_H_
#define _MIWA_SYSTEM_H_

#include <stdint.h>

#define SYS_MAX_NAME	255

#ifdef __cplusplus
extern "C" {
#endif

void sys_init(const char *name);

int sys_is_big_endian(void);

int sys_file_exists(const char *file);
int sys_directory_exists(const char *dir);
int sys_create_directory(const char *path);

const char *sys_os_name(void);
const char *sys_os_version(void);
const char *sys_machine(void);
const char *sys_hostname(void);

const char *sys_cpu_name(void);
uint32_t sys_cpu_freq(void);
uint32_t sys_cpu_count(void);

uint64_t sys_mem_used(void);
uint64_t sys_mem_free(void);
uint64_t sys_mem_total(void);

void sys_sleep(uint32_t sec);
void sys_msleep(uint32_t msec);
void sys_usleep(uint32_t usec);

void *sys_load_library(const char *library);
void *sys_get_proc_address(void *library, const char *proc);
void sys_unload_library(void *library);

const char *sys_log_dir(void);
const char *sys_temp_dir(void);
const char *sys_cache_dir(void);
const char *sys_runtime_dir(void);

int platform_init(void);
void platform_release(void);

#ifdef __cplusplus
}
#endif

#endif /* _MIWA_SYSTEM_H_ */

