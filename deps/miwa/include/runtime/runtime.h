/* Miwa Portable Runtime
 *
 * runtime.h
 * Author: Alexandru Naiman
 *
 * Runtime header
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

#ifndef _MIWA_RUNTIME_H_
#define _MIWA_RUNTIME_H_

#include <stdint.h>
#include <stdlib.h>

#include <system/defs.h>
#include <runtime/array.h>
#include <runtime/queue.h>
#include <runtime/string.h>
#include <runtime/variant.h>

#define RT_NOT_FOUND	(size_t)-1

#ifdef __cplusplus
extern "C" {
#endif

int rt_is_numeric(const char *str);

void rt_swap_int16(int16_t *val);
void rt_swap_int32(int32_t *val);
void rt_swap_int64(int64_t *val);
void rt_swap_uint16(uint16_t *val);
void rt_swap_uint32(uint32_t *val);
void rt_swap_uint64(uint64_t *val);

#ifdef __cplusplus
}
#endif

#endif /* _MIWA_RUNTIME_H_ */

