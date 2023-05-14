#ifndef NE_RUNTIME_STRING_H
#define NE_RUNTIME_STRING_H

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <System/System.h>
#include <System/Memory.h>
#include <Runtime/RtDefs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * struct NeString - string
 * @data:
 * @count:
 * @size:
 * @heap
 */
struct NeString
{
	uint64_t hash;
	char *data;
	size_t count;
	size_t size;
	enum NeMemoryHeap heap;
	bool ownMemory;
};

/**
 * Rt_InitString - initialize a string
 * @a: the structure to initialize
 * @size: initial array size (in elements)
 * @heap: size of one array element
 */
static inline bool
Rt_InitString(struct NeString *s, size_t size, enum NeMemoryHeap heap)
{
	if (!s || !size)
		return false;

	Sys_ZeroMemory(s, sizeof(*s));

	s->data = (char *)Sys_Alloc(sizeof(*s->data), size, heap);
	if (!s->data)
		return false;

	s->count = 0;
	s->size = size;
	s->heap = heap;
	s->ownMemory = true;

	return true;
}

/**
 * Rt_InitString - initialize a string with a c string
 * @a: the structure to initialize
 * @size: initial array size (in elements)
 * @heap: size of one array element
 */
static inline bool
Rt_StringWithCStringN(struct NeString *s, enum NeMemoryHeap heap, const char *cstr, size_t len)
{
	if (!s || !cstr)
		return false;

	Sys_ZeroMemory(s, sizeof(*s));

	s->data = (char *)Sys_Alloc(sizeof(*s->data), len + 1, heap);
	if (!s->data)
		return false;

	strlcpy(s->data, cstr, len + 1);

	s->count = len;
	s->size = len + 1;
	s->heap = heap;
	s->hash = Rt_HashString(s->data);
	s->ownMemory = true;

	return true;
}

static inline bool
Rt_StringWithCString(struct NeString *s, enum NeMemoryHeap heap, const char *cstr)
{
	return Rt_StringWithCStringN(s, heap, cstr, strnlen(cstr, UINT32_MAX));
}

static inline bool
Rt_StringWithExistingMemory(struct NeString *s, char *cstr)
{
	if (!s || !cstr)
		return false;

	Sys_ZeroMemory(s, sizeof(*s));

	size_t len = strnlen(cstr, UINT32_MAX);

	s->data = cstr;
	s->count = len;
	s->size = len + 1;
	s->heap = MH_System;
	s->hash = Rt_HashString(cstr);
	s->ownMemory = false;

	return true;
}

static inline bool
Rt_StringWithFormat(struct NeString *s, enum NeMemoryHeap heap, const char *fmt, ...)
{
	va_list list;

	if (!s || !fmt)
		return false;

	Sys_ZeroMemory(s, sizeof(*s));

	va_start(list, fmt);
	s->size = vsnprintf(s->data, 0, fmt, list) + 1;

	s->data = (char *)Sys_Alloc(sizeof(*s->data), s->size, heap);
	if (!s->data) {
		va_end(list);
		return false;
	}

	vsnprintf(s->data, s->size, fmt, list);
	va_end(list);

	s->count = strnlen(s->data, s->size);
	s->heap = heap;
	s->hash = Rt_HashString(s->data);
	s->ownMemory = true;

	return true;
}

/**
 * Rt_CloneString - clone a string
 * @dst: destination array
 * @src: the array to clone
 *
 * Return: 0 on success
 */
static inline bool
Rt_CloneString(struct NeString *dst, const struct NeString *src, enum NeMemoryHeap heap)
{
	if (!dst || !src)
		return false;

	char *data = (char *)Sys_Alloc(sizeof(*dst->data), src->size, heap);
	if (!data)
		return false;

	memcpy(data, src->data, src->size * sizeof(*dst->data));
	memcpy(dst, src, sizeof(*dst));
	dst->data = data;
	dst->heap = heap;
	dst->ownMemory = true;

	return true;
}

/**
 * Rt_StringFirst - get the first character of a string
 * @s: the string
 *
 * Returns: first character of the string
 */
#define Rt_StringFirst(s) (s)->data

/**
 * Rt_StringLast - get the last character of a string
 * @a: the string
 *
 * Returns: last character of the string
 */
#define Rt_StringLast(s) ((s)->data + sizeof(*(s)->data) * ((s)->count - 1))

/**
 * Rt_StringCount / Rt_StringLength - get the string length
 * @s: the string
 *
 * Returns: number of characters
 */
#define Rt_StringCount(s) (s)->count
#define Rt_StringLength(s) (s)->count

/**
 * Rt_StringSize - get the size of the string
 * @s: the string
 *
 * Returns: string size (including nul terminator)
 */
#define Rt_StringSize(s) (s)->size

/**
 * Rt_StringByteSize - get the size in bytes of a string
 * @s: the string
 *
 * Returns: size in bytes of the string
 */
#define Rt_StringByteSize(s) ((s)->size * sizeof((s)->data))

/**
 * Rt_StringUsedByteSize - get the used size in bytes of a string
 * @s: the string
 *
 * Returns: used size in bytes of the string
 */
#define Rt_StringUsedByteSize(s) ((s)->count * sizeof((s)->data))

/**
 * Rt_StringDataPtr - get the data pointer of an array
 * @s: the string
 *
 * Returns: data pointer
 */
#define Rt_StringDataPtr(s) (s)->data

static inline int
Rt_ResizeString(struct NeString *s, size_t size)
{
	char *ptr = s->data;

	if (!s->ownMemory)
		return false;

	if (s->size == size)
		return true;

	if ((s->data = (char *)Sys_ReAlloc(s->data, sizeof(*s->data), size, s->heap)) == NULL) {
		s->data = ptr;
		return false;
	}

	s->size = size;

	if (s->size < s->count - 1)
		s->count = s->size - 1;

	// ensure nul terminator
	*(Rt_StringLast(s) + 1) = 0x0;

	return true;
}

static inline bool
Rt_StringSetN(struct NeString *s, const char *cstr, size_t len)
{
	if (s->size < len + 1)
		if (!Rt_ResizeString(s, _Rt_CalcGrowSize(s->size, sizeof(*s->data), len + 1)))
			return false;

	Sys_ZeroMemory(s->data, s->size * sizeof(*s->data));
	strlcpy(s->data, cstr, len + 1);

	s->count = len;
	s->hash = Rt_HashString(s->data);

	return true;
}

static inline bool
Rt_StringSet(struct NeString *s, const char *cstr)
{
	return Rt_StringSetN(s, cstr, strnlen(cstr, UINT16_MAX));
}

/**
 * Rt_StringAppend - add an item to the array
 * @a: the array
 * @item: item to add
 *
 * &rt_array.elemSize number of bytes will be copied from item to the next
 * location in the array. If &rt_array.count is equal to &rt_array.size the
 * array will grow.
 *
 * Returns: OK on success
 */
static inline bool
Rt_StringAppend(struct NeString *s, char c)
{
	if (s->count == s->size - 1)
		if (!Rt_ResizeString(s, _Rt_CalcGrowSize(s->size, sizeof(*s->data), s->size + RT_DEF_INC)))
			return false;

	s->data[s->count++] = c;
	s->hash = Rt_HashString(s->data);

	return true;
}

/**
 * Rt_StringAppendString - add the contents of an array to the array
 * @a: the array
 * @item: the array to add
 *
 * &rt_array.elemSize number of bytes will be copied from item to the next
 * location in the array. If &rt_array.count is equal to &rt_array.size the
 * array will grow.
 *
 * Returns: OK on success
 */
static inline bool
Rt_StringAppendString(struct NeString *s, const struct NeString *str)
{
	if (s->count + str->count >= s->size - 1)
		if (!Rt_ResizeString(s, _Rt_CalcGrowSize(s->size, sizeof(*s->data), (s->size + str->count) + RT_DEF_INC)))
			return false;

	strlcat(s->data, str->data, s->size);
	s->count += str->count;
	s->hash = Rt_HashString(s->data);

	return true;
}

static inline bool
Rt_StringAppendCStringN(struct NeString *s, const char *cstr, size_t len)
{
	if (s->count + len >= s->size - 1)
		if (!Rt_ResizeString(s, _Rt_CalcGrowSize(s->size, sizeof(*s->data), (s->size + len) + RT_DEF_INC)))
			return false;

	strlcat(s->data, cstr, s->size);
	s->count += len;
	s->hash = Rt_HashString(s->data);

	return true;
}

static inline bool
Rt_StringAppendCString(struct NeString *s, const char *cstr)
{
	return Rt_StringAppendCStringN(s, cstr, strnlen(cstr, UINT32_MAX));
}

static inline bool
Rt_StringAppendFormat(struct NeString *s, const char *fmt, ...)
{
	va_list list;

	if (!s || !fmt)
		return false;

	va_start(list, fmt);
	size_t len = vsnprintf(NULL, 0, fmt, list) + 1;

	char *tmp = (char *)Sys_Alloc(sizeof(*tmp), len, MH_Transient);
	if (!tmp) {
		va_end(list);
		return false;
	}

	vsnprintf(tmp, len, fmt, list);
	va_end(list);

	return Rt_StringAppendCStringN(s, tmp, len - 1);
}

/**
 * Rt_StringInsert - insert a string into the string
 * @s: the string
 * @cstr: string to add
 * @len: c string length
 * @pos: position to insert the string at
 *
 * &rt_array.elemSize number of bytes will be copied from item to the @pos
 * location in the array. If &rt_array.count is equal to &rt_array.size the
 * array will grow.
 *
 * Returns: OK on success
 */
static inline bool
Rt_StringInsert(struct NeString *s, const char *str, size_t len, size_t pos)
{
	if (s->count + len == s->size - 1)
		if (Rt_ResizeString(s, _Rt_CalcGrowSize(s->size, sizeof(*s->data), s->size + len + RT_DEF_INC)))
			return false;

	for (size_t i = s->count; i > pos; --i)
		s->data[i] = s->data[i - 1];

	for (size_t i = pos; i < pos + len; ++i)
		s->data[i] = str[i - pos];

	s->count += len;

	*(Rt_StringLast(s) + 1) = 0x0;
	s->hash = Rt_HashString(s->data);

	return true;
}

static inline bool
Rt_StringRemove(struct NeString *s, size_t index)
{
	if (!s->count || index >= s->count)
		return false;

	--s->count;

	for (size_t i = index + 1; i <= s->count; ++i)
		s->data[i - 1] = s->data[i];

	*(Rt_StringLast(s) + 1) = 0x0;
	s->hash = Rt_HashString(s->data);

	return true;
}

static inline size_t
Rt_StringFirstIndexOf(const struct NeString *s, char c)
{
	char *ptr = strchr(s->data, c);
	if (!ptr)
		return RT_NOT_FOUND;
	return ptr - s->data;
}

static inline size_t
Rt_StringLastIndexOf(const struct NeString *s, char c)
{
	char *ptr = strrchr(s->data, c);
	if (!ptr)
		return RT_NOT_FOUND;
	return ptr - s->data;
}

static inline bool
Rt_StringReverse(struct NeString *s)
{
	if (!s->count)
		return false;

	for (uint64_t i = 0; i < s->count; ++i) {
		s->data[i] ^= s->data[s->count - i];
		s->data[s->count - i] ^= s->data[i];
		s->data[i] ^= s->data[s->count - i];
	}

	s->hash = Rt_HashString(s->data);

	return true;
}

static inline struct NeArray *
Rt_StringSplit(const struct NeString *s, char delim, enum NeMemoryHeap heap)
{
	size_t elements = 1;
	for (size_t i = 0; i < s->count; ++i)
		if (s->data[i] == delim)
			++elements;

	struct NeArray *dst = (struct NeArray *)Sys_Alloc(sizeof(*dst), 1, heap);
	Rt_InitArray(dst, elements, sizeof(struct NeString), heap);

	struct NeString tmp;
	Rt_CloneString(&tmp, s, heap);

	if (elements == 1) {
		Rt_ArrayAdd(dst, &tmp);
	} else {
		char *start = tmp.data;
		char *ptr = strchr(tmp.data, delim);

		while (ptr) {
			struct NeString stmp;
			Rt_StringWithCStringN(&stmp, heap, start, ptr - start);
			Rt_ArrayAdd(dst, &stmp);

			start = ++ptr;
			if (ptr)
				ptr = strchr(++ptr, delim);
		}

		if (!start)
			return dst;

		struct NeString stmp;
		Rt_StringWithCString(&stmp, heap, start);
		Rt_ArrayAdd(dst, &stmp);
	}

	return dst;
}

#define Rt_FillString(s) (s)->count = (s)->size
#define Rt_ZeroString(s) memset((s)->data, 0x0, sizeof(*(s)->data) * (s)->count); (s)->count = 0

static inline void
Rt_ClearString(struct NeString *s, bool freeMemory)
{
	s->count = 0;

	if (!s->ownMemory || !freeMemory || !s->data)
		return;

	s->size = 0;
	s->hash = 0;
	Sys_Free(s->data);
	s->data = NULL;
}

static inline void
Rt_TermString(struct NeString *s)
{
	if (!s)
		return;

	Rt_ClearString(s, true);
}

#ifdef __cplusplus
}
#endif

#endif /* NE_RUNTIME_STRING_H */

/* NekoEngine
 *
 * String.h
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
