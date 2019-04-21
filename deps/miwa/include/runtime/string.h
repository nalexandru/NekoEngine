/* Miwa Portable Runtime
 *
 * string.h
 * Author: Alexandru Naiman
 *
 * String
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

#ifndef _MIWA_RUNTIME_STRING_H_
#define _MIWA_RUNTIME_STRING_H_

#include <string.h>
#include <stdarg.h>

#include <runtime/array.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rt_string
{
	char *data;
	size_t length;
	size_t size;
	uint64_t hash;
} rt_string;

int rt_string_init(rt_string *s, size_t size);
int rt_string_init_with_cstr(rt_string *s, const char *str);
int rt_string_init_with_format(rt_string *s, size_t size, const char *fmt, ...);
rt_string *rt_string_clone(rt_string *s);

bool rt_string_is_empty(rt_string *s);
bool rt_string_is_equal(rt_string *s1, rt_string *s2);
bool rt_string_is_equal_to_cstr(rt_string *s1, const char *s2);

int rt_string_compare(rt_string *s1, rt_string *s2);
int rt_string_compare_to_cstr(rt_string *s1, const char *s2);

int rt_string_append(rt_string *s, const char *str);
int rt_string_append_format(rt_string *s, const char *fmt, ...);
int rt_string_resize(rt_string *s, size_t size);

uint64_t rt_string_find(rt_string *s, const char *str);

uint64_t rt_string_find_first(rt_string *s, char c);
uint64_t rt_string_find_last(rt_string *s, char c);

rt_string *rt_string_substring(rt_string *s, size_t start, size_t end);

rt_array *rt_string_split(rt_string *s, char delim);

void rt_string_trim(rt_string *s);
void rt_string_remove_new_line(rt_string *s);

void rt_string_clear(rt_string *s, bool free_memory);

void rt_string_release(rt_string *s);

uint64_t rt_hash_string(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* _MIWA_RUNTIME_STRING_H_ */

