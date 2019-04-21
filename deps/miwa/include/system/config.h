/* Miwa Portable Runtime
 *
 * config.h
 * Author: Alexandru Naiman
 *
 * Configuration System
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

#ifndef _SYSTEM_CONFIG_H_
#define _SYSTEM_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

#include <system/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Return value:
 * 0 - OK
 * 1 - File read error
 * 2 - File not found
 */
uint8_t sys_config_init(const char *file);

void sys_config_set_int(const char *key, int64_t val);
void sys_config_set_bool(const char *key, bool val);
void sys_config_set_double(const char *key, double val);
void sys_config_set_string(const char *key, const char *val);

int64_t sys_config_get_int(const char *key, int64_t def);
bool sys_config_get_bool(const char *key, bool def);
double sys_config_get_double(const char *key, float def);
const char *sys_config_get_string(const char *key, const char *def);

bool sys_config_save(const char *file);
uint8_t sys_config_reload(void);

void sys_config_release(void);

#ifdef __cplusplus
}
#endif

#endif /* _SYSTEM_CONFIG_H_ */

