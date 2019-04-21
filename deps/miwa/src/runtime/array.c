/* Miwa Portable Runtime
 *
 * array.c
 * Author: Alexandru Naiman
 *
 * Array
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

#include <assert.h>

#include <system/compat.h>
#include <runtime/runtime.h>

#include "rt_internal.h"

int
rt_array_init(rt_array *a,
	size_t size,
	size_t elem_size)
{
	if (!a || !size || !elem_size)
		return SYS_INVALID_ARGS;

	memset(a, 0x0, sizeof(rt_array));

	a->data = calloc(size, elem_size);

	if (!a->data)
		return SYS_MEMORY;

	a->count = 0;
	a->size = size;
	a->elem_size = elem_size;

	return SYS_OK;
}

int
rt_array_init_ptr(rt_array *a,
	size_t size)
{
	return rt_array_init(a, size, sizeof(void *));
}

int
rt_array_clone(rt_array *dst,
	const rt_array *src)
{
	if (!dst)
		return SYS_INVALID_ARGS;

	dst->data = calloc(src->size, src->elem_size);
	if (!dst->data)
		return SYS_MEMORY;

	memcpy(dst->data, src->data, src->size * src->elem_size);
		
	dst->size = src->size;
	dst->count = src->count;
	dst->elem_size = src->elem_size;

	return SYS_OK;
}

int
rt_array_add(rt_array *a,
	const void *data)
{
	int ret = SYS_OK;

	if (a->count == a->size)
		if ((ret = rt_array_resize(a, _rt_calc_grow_size(a->size,
			a->elem_size, a->size + RT_DEF_INC))) != SYS_OK)
			return ret;

	memcpy(a->data + a->elem_size * a->count++, data, a->elem_size);

	return SYS_OK;
}

int
rt_array_add_ptr(rt_array *a,
	const void *data)
{
	return rt_array_add(a, &data);
}

void *
rt_array_create(rt_array *a)
{
	void *ptr = NULL;

	if (a->count == a->size)
		if (rt_array_resize(a, _rt_calc_grow_size(a->size,
			a->elem_size, a->size + RT_DEF_INC)) != SYS_OK)
			return NULL;

	ptr = a->data + a->elem_size * a->count++;
	memset(ptr, 0x0, a->elem_size);

	return ptr;
}

static INLINE int
_array_insert(rt_array *a,
	const void *data,
	size_t pos,
	bool ordered)
{
	int ret = SYS_OK;
	size_t i = 0;

	if (a->count == a->size)
		if ((ret = rt_array_resize(a, _rt_calc_grow_size(a->size,
			a->elem_size, a->size + RT_DEF_INC))) != SYS_OK)
			return ret;

	if (!ordered) {
		memcpy(a->data + a->elem_size * a->count, rt_array_get(a, pos), a->elem_size);
	} else {
		for (i = a->count; i > pos; --i)
			memcpy(a->data + a->elem_size * i, a->data + a->elem_size * (i - 1), a->elem_size);
	}

	memcpy(a->data + a->elem_size * pos, data, a->elem_size);
	++a->count;

	return SYS_OK;
}

int
rt_array_insert(rt_array *a,
	const void *item,
	size_t pos)
{
	return _array_insert(a, item, pos, true);
}

int
rt_array_insert_ptr(rt_array *a,
	const void* item,
	size_t pos)
{
	return _array_insert(a, &item, pos, true);
}

int
rt_array_fast_insert(rt_array *a,
	const void *item,
	size_t pos)
{
	return _array_insert(a, item, pos, false);
}

int
rt_array_fast_insert_ptr(rt_array *a,
	const void *item,
	size_t pos)
{
	return _array_insert(a, &item, pos, false);
}

int
rt_array_remove(rt_array *a,
	size_t index)
{
	size_t i = 0;

	if (index > a->count - 1)
		return SYS_NOT_FOUND;

	--a->count;

	for (i = index + 1; i <= a->count; ++i)
		memcpy(a->data + a->elem_size * (i - 1), a->data + a->elem_size * i, a->elem_size);

	return SYS_OK;
}

int
rt_array_resize(rt_array *a,
	size_t size)
{
	uint8_t *ptr = a->data;

	if (a->size == size)
		return SYS_OK;
	
	if ((a->data = (uint8_t *)reallocarray(a->data, size, a->elem_size)) == NULL) {
		a->data = ptr;
		return SYS_MEMORY;
	}

	a->size = size;

	if (a->size < a->count)
		a->count = a->size;

	return SYS_OK;
}

void *
rt_array_get(const rt_array *a,
	size_t id)
{
	if (id > a->size)
		return NULL;
	
	return a->data + a->elem_size * id;
}

void *
rt_array_get_ptr(const rt_array *a,
	size_t id)
{
	if (id > a->size)
		return NULL;
	
	return *(void **)rt_array_get(a, id);
}

void *
rt_array_find(const rt_array *a,
	const void *data,
	int32_t (*cmpfunc)(const void *item, const void *data))
{
	size_t i;

	for (i = 0; i < a->count; ++i)
		if (!cmpfunc(a->data + a->elem_size * i, data))
			return a->data + a->elem_size * i;

	return NULL;
}

size_t
rt_array_find_id(const rt_array *a,
	const void *data,
	int32_t (*cmpfunc)(const void *item, const void *data))
{
	size_t i;

	for (i = 0; i < a->count; ++i)
		if (!cmpfunc(a->data + a->elem_size * i, data))
			return i;

	return RT_NOT_FOUND;
}

void *
rt_array_bsearch(const rt_array *a,
	const void *data,
	int32_t (*cmpfunc)(const void *item, const void *data))
{
	size_t start = 0, end = a->count - 1, mid;
	int32_t c;
	void *elem = 0;

	if (!a->count)
		return NULL;

	while (start <= end) {
		mid = start + (end - start) / 2;

		elem = a->data + a->elem_size * mid;

		c = cmpfunc(elem, data);
		if (!c)
			return elem;
		else if (c < 0)
			start = mid + 1;
		else
			end = mid - 1;
	}

	return NULL;
}

size_t
rt_array_bsearch_id(const rt_array *a,
	const void *data,
	int32_t (*cmpfunc)(const void *item, const void *data))
{
	size_t start = 0, end = a->count - 1, mid;
	int32_t c;
	void *elem = 0;

	if (!a->count)
		return RT_NOT_FOUND;

	while (start <= end) {
		mid = start + (end - start) / 2;

		elem = a->data + a->elem_size * mid;

		c = cmpfunc(elem, data);
		if (!c)
			return mid;
		else if (c < 0)
			start = mid + 1;
		else
			end = mid - 1;
	}

	return RT_NOT_FOUND;
}

void
rt_array_sort(rt_array *a,
	int32_t (*sortfunc)(const void *a, const void *b))
{
	qsort(a->data, a->count, a->elem_size, sortfunc);
}

void
rt_array_reverse(rt_array *a)
{
	uint8_t *tmp = NULL;
	uint64_t s = 0, e = 0;
	void *start = 0, *end = 0;

	if (!a->count)
		return;

	tmp = calloc(1, a->elem_size);
	assert(tmp);

	s = 0;
	e = a->count - 1;

	while (s < e) {
		start = a->data + a->elem_size * s++;
		end = a->data + a->elem_size * e--;

		memcpy(tmp, start, a->elem_size);
		memcpy(start, end, a->elem_size);
		memcpy(end, tmp, a->elem_size);
	}

	free(tmp);
}

size_t
rt_array_upper_bound(const rt_array *a,
	const void *data,
	int32_t (*cmpfunc)(const void *item, const void *data))
{
	size_t low = 0, mid = 0;
	size_t high = a->count;

	while (low < high) {
		mid = (low + high) / 2;

		if (cmpfunc(a->data + a->elem_size * mid, data) < 0)
			high = mid;
		else
			low = mid + 1;
	}

	return low;
}

size_t
rt_array_lower_bound(const rt_array *a,
	const void *data,
	int32_t (*cmpfunc)(const void *item, const void *data))
{
	size_t low = 0, mid = 0;
	size_t high = a->count;

	while (low < high) {
		mid = (low + high) / 2;

		if (cmpfunc(a->data + a->elem_size * mid, data) >= 0)
			high = mid;
		else
			low = mid + 1;
	}

	return low;
}

void
rt_array_clear(rt_array *a,
	bool free_memory)
{
	a->count = 0;

	if (!free_memory)
		return;

	a->size = 0;
	free(a->data);
	a->data = NULL;
}

void
rt_array_release(rt_array *a)
{
	if (!a)
		return;

	rt_array_clear(a, true);
}

