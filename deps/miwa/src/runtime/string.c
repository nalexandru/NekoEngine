/* Miwa Portable Runtime
 *
 * string.c
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

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>

#include <system/compat.h>
#include <runtime/runtime.h>

int
rt_string_init(rt_string *s,
	size_t size)
{
	if (!s)
		return SYS_INVALID_ARGS;

	s->data = calloc(size, sizeof(char));
	if (!s->data)
		return SYS_MEMORY;

	s->length = 0;
	s->hash = 0;
	s->size = size;

	return SYS_OK;
}

int
rt_string_init_with_cstr(rt_string *s,
	const char *str)
{
	int ret = rt_string_init(s, strlen(str) + 1);
	if (ret != SYS_OK)
		return ret;

	snprintf(s->data, s->size, "%s", str);
	s->length = s->size - 1;
	s->hash = rt_hash_string(s->data);

	return SYS_OK;
}

int
rt_string_init_with_format(rt_string *s,
	size_t size,
	const char *fmt,
	...)
{
	va_list args;
	int ret = rt_string_init(s, size);
	if (ret != SYS_OK)
		return ret;

	va_start(args, fmt);
	vsnprintf(s->data, s->size, fmt, args);
	va_end(args);

	s->length = strlen(s->data);
	s->hash = rt_hash_string(s->data);

	return SYS_OK;
}

rt_string *
rt_string_clone(rt_string *s)
{
	int ret = 0;
	rt_string *new = calloc(1, sizeof(rt_string));

	if (!new)
		return NULL;

	ret = rt_string_init(new, s->size);
	if (ret != SYS_OK)
		goto free_new;

	new->data = strdup(s->data);
	if (!new->data)
		goto destroy_new;

	new->length = s->length;
	new->hash = s->hash;

	return new;

destroy_new:
	rt_string_release(new);

free_new:
	free(new);

	return NULL;
}

bool
rt_string_is_empty(rt_string *s)
{
	return s->length == 0 || s->data[0] == 0x0;
}

bool
rt_string_is_equal(rt_string *s1,
	rt_string *s2)
{
	if (!s1->hash)
		s1->hash = rt_hash_string(s1->data);

	if (!s2->hash)
		s2->hash = rt_hash_string(s2->data);

	return s1->hash == s2->hash;
}

bool
rt_string_is_equal_to_cstr(rt_string *s1,
	const char *s2)
{
	uint64_t hash = rt_hash_string(s2);
	
	if (!s1->hash)
		s1->hash = rt_hash_string(s1->data);
	
	return s1->hash == hash;
}

int
rt_string_compare(rt_string *s1,
	rt_string *s2)
{
	if (rt_string_is_equal(s1, s2))
		return 0;

	return strncmp(s1->data, s2->data, s1->size);
}

int
rt_string_compare_to_cstr(rt_string *s1,
	const char *s2)
{
	if (rt_string_is_equal_to_cstr(s1, s2))
		return 0;
	
	return strncmp(s1->data, s2, s1->length);
}

int
rt_string_append(rt_string *s,
	const char *str)
{
	size_t len = 0;
	int ret = SYS_ERROR;

	if (!str)
		return SYS_INVALID_ARGS;

	len = strlen(str);

	if (!len)
		return SYS_INVALID_ARGS;

	if (!s->data) {
		s->size = len + 1;
		s->length = len;
		if ((s->data = (char *)calloc(1, s->size)) == NULL)
			return SYS_MEMORY;

		memcpy(s->data, str, len);
		s->data[len] = 0x0;
		s->hash = rt_hash_string(s->data);

		return SYS_OK;
	}

	if (s->length + len >= s->size) {
		ret = rt_string_resize(s, s->length + len + 1);
		if (ret != SYS_OK)
			return ret;
	}

	strncat(s->data, str, len);
	s->length += len;
	s->data[s->length] = 0x0;
	s->hash = rt_hash_string(s->data);

	return SYS_OK;
}

int
rt_string_append_format(rt_string *s,
	const char *fmt,
	...)
{
	/*NString str(len);
	va_list args;

	va_start(args, fmt);
	vsnprintf(*str, len, fmt, args);
	va_end(args);

	str._length = strlen(*str);

	Append(str);*/

	return SYS_ERROR;
}

int
rt_string_resize(rt_string *s,
	size_t size)
{
	char *ptr = 0;
	if (s->size == size)
		return SYS_OK;

	ptr = s->data;
	if ((s->data = (char *)reallocarray(s->data, size, sizeof(char))) == NULL) {
		s->data = ptr;
		return SYS_MEMORY;
	}

	s->size = size;

	if (s->size < s->length) {
		s->length = s->size - 1;
		s->data[s->length] = 0x0;
		s->hash = rt_hash_string(s->data);
	}

	return SYS_OK;
}

uint64_t
rt_string_find(rt_string *s,
	const char *str)
{
	char *ptr = strstr(s->data, str);

	if (!ptr)
		return RT_NOT_FOUND;

	return ptr - s->data;
}

uint64_t
rt_string_find_first(rt_string *s,
	char c)
{
	char *ptr = strchr(s->data, c);

	if (!ptr)
		return RT_NOT_FOUND;

	return ptr - s->data;
}

uint64_t
rt_string_find_last(rt_string *s,
	char c)
{
	char *ptr = strrchr(s->data, c);

	if (!ptr)
		return RT_NOT_FOUND;

	return ptr - s->data;
}

rt_string *
rt_string_substring(rt_string *s,
	size_t start,
	size_t end)
{
	return NULL;
}

rt_array *
rt_string_split(rt_string *s,
	char delim)
{
	return NULL;
}

void
rt_string_trim(rt_string *s)
{
	uint64_t i;

	for (i = s->length; i > 0; --i) {
		if (s->data[i] == ' ' || s->data[i] == '\r' || s->data[i] == '\n') {
			s->data[i] = 0x0;
		} else if (s->data[i] != 0x0) {
			goto exit;
		}
	}

exit:
	s->length = strlen(s->data);
	s->hash = rt_hash_string(s->data);
}

void
rt_string_remove_new_line(rt_string *s)
{
	uint64_t i;

	for (i = s->length; i > 0; --i) {
		if (s->data[i] == '\n' || s->data[i] == '\r') {
			s->data[i] = 0x0;
		} else if (s->data[i] != 0x0 && s->data[i] != ' ') {
			goto exit;
		}
	}

exit:
	s->length = strlen(s->data);
	s->hash = rt_hash_string(s->data);
}

void
rt_string_clear(rt_string *s,
	bool free_memory)
{
	explicit_bzero(s->data, s->size);

	s->hash = 0;
	s->length = 0;

	if (!free_memory)
		return;
	
	s->size = 0;
	free(s->data);
	s->data = NULL;
}

void
rt_string_release(rt_string *s)
{
	rt_string_clear(s, true);
}

uint64_t
rt_hash_string(const char *str)
{
	uint64_t hash = 0;

	while (*str) {
		hash += *str++;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

