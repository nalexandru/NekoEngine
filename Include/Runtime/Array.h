#ifndef _NE_RUNTIME_ARRAY_H_
#define _NE_RUNTIME_ARRAY_H_

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include <System/System.h>
#include <System/Memory.h>
#include <Runtime/RtDefs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * struct NeArray - basic array
 * @data:
 * @count:
 * @size:
 * @elemSize
 * @heap
 */
struct NeArray
{
	uint8_t *data;
	size_t count;
	size_t size;
	size_t elemSize;
	size_t align;
	enum NeMemoryHeap heap;
};

/**
 * Rt_InitArray - initialize a rt_array structure
 * @a: the structure to initialize
 * @size: initial array size (in elements)
 * @elemSize: size of one array element
 */
static inline bool
Rt_InitArray(struct NeArray *a, size_t size, size_t elemSize, enum NeMemoryHeap heap)
{
	if (!a || !size || !elemSize)
		return false;

	Sys_ZeroMemory(a, sizeof(*a));

	a->data = (uint8_t *)Sys_Alloc(size, elemSize, heap);
	if (!a->data)
		return false;

	a->count = 0;
	a->size = size;
	a->elemSize = elemSize;
	a->align = 1;
	a->heap = heap;

	return true;
}

/**
 * Rt_InitAlignedArray - initialize a rt_array structure with
 * data aligned to the specified boundary. The array will
 * have padding between items if necessary.
 * @a: the structure to initialize
 * @size: initial array size (in elements)
 * @elemSize: size of one array element
 */
static inline bool
Rt_InitAlignedArray(struct NeArray *a, size_t size, size_t elemSize, size_t alignment)
{
	if (!a || !size || !elemSize)
		return false;

	if (alignment == 1)
		return Rt_InitArray(a, size, elemSize, MH_System);

	memset(a, 0x0, sizeof(*a));

	a->data = (uint8_t *)aligned_alloc(alignment, size * elemSize);
	if (!a->data)
		return false;

	a->count = 0;
	a->size = size;
	a->elemSize = (elemSize + alignment - 1) & ~(alignment - 1);
	a->align = alignment;
	a->heap = MH_ManualAlign;

	return true;
}

/**
 * Rt_InitPtrArray - initialize a rt_array structure for pointers
 * @a: the structure to initialize
 * @size: initial array size (in elements)
 */
#define Rt_InitPtrArray(a, size, heap) Rt_InitArray(a, size, sizeof(void *), heap)

/**
 * Rt_CloneArray - clone an array
 * @dst: destination array
 * @src: the array to clone
 *
 * Return: 0 on success
 */
static inline bool
Rt_CloneArray(struct NeArray *dst, const struct NeArray *src, enum NeMemoryHeap heap)
{
	if (!dst || !src)
		return false;

	uint8_t *data = (uint8_t *)Sys_Alloc(src->size, src->elemSize, heap);
	if (!data)
		return false;

	memcpy(data, src->data, src->size * src->elemSize);
	memcpy(dst, src, sizeof(*dst));
	dst->data = data;
	dst->heap = heap;

	return true;
}

/**
 * Rt_ArrayFirst - get the first element of an array
 * @a: the array
 *
 * Returns: first element of the array
 */
#define Rt_ArrayFirst(a) (a)->data

/**
 * Rt_ArrayLast - get the last element of an array
 * @a: the array
 *
 * Returns: last element of the array
 */
#define Rt_ArrayLast(a) ((a)->data + (a)->elemSize * ((a)->count - 1))

/**
 * Rt_ArrayCount - get the number of elements in an array
 * @a: the array
 *
 * Returns: number of elements
 */
#define Rt_ArrayCount(a) (a)->count

/**
 * Rt_ArraySize - get the size in elements of an array
 * @a: the array
 *
 * Returns: size in elements of the array
 */
#define Rt_ArraySize(a) (a)->size

/**
 * Rt_ArrayByteSize - get the size in bytes of an array
 * @a: the array
 *
 * Returns: size in bytes of the array
 */
#define Rt_ArrayByteSize(a) ((a)->size * (a)->elemSize)

/**
 * Rt_ArrayUsedByteSize - get the used size in bytes of an array
 * @a: the array
 *
 * Returns: used size in bytes of the array
 */
#define Rt_ArrayUsedByteSize(a) ((a)->count * (a)->elemSize)

/**
 * Rt_ArrayDataPtr - get the data pointer of an array
 * @a: the array
 *
 * Returns: data pointer
 */
#define Rt_ArrayDataPtr(a) (a)->data

static inline int
Rt_ResizeArray(struct NeArray *a, size_t size)
{
	uint8_t *ptr = a->data;

	if (a->size == size)
		return true;

	if (a->align > 1) {
		if ((a->data = (uint8_t *)aligned_alloc(a->align, size * a->elemSize)) == NULL) {
			a->data = ptr;
			return false;
		}

		memcpy(a->data, ptr, Rt_ArrayUsedByteSize(a));
#ifndef SYS_PLATFORM_WINDOWS
		free(ptr);
#else
		_aligned_free(ptr);
#endif
	} else {
		if ((a->data = (uint8_t *)Sys_ReAlloc(a->data, size, a->elemSize, a->heap)) == NULL) {
			a->data = ptr;
			return false;
		}
	}

	a->size = size;

	if (a->size < a->count)
		a->count = a->size;

	return true;
}

/**
 * Rt_ArrayAdd - add an item to the array
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
Rt_ArrayAdd(struct NeArray *a, const void *data)
{
	if (a->count == a->size)
		if (!Rt_ResizeArray(a, _Rt_CalcGrowSize(a->size, a->elemSize, a->size + RT_DEF_INC)))
			return false;

	memcpy(a->data + a->elemSize * a->count++, data, a->elemSize);

	return true;
}

/**
 * Rt_ArrayAddPtr - add a pointer to the array
 * @a: the array
 * @item: item to add
 *
 * Convenience function for arrays of pointers. Behaves exactly like
 * rt_array_add()
 *
 * Returns: OK on success
 */
static inline bool
Rt_ArrayAddPtr(struct NeArray *a, const void *data)
{
	return Rt_ArrayAdd(a, &data);
}

/**
 * Rt_ArrayAddArray - add the contents of an array to the array
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
Rt_ArrayAddArray(struct NeArray *a, const struct NeArray *src)
{
	if (a->count + src->count >= a->size)
		if (!Rt_ResizeArray(a, _Rt_CalcGrowSize(a->size, a->elemSize, (a->size + src->count) + RT_DEF_INC)))
			return false;

	memcpy(a->data + a->elemSize * a->count, src->data, a->elemSize * src->count);
	a->count += src->count;

	return true;
}

/**
 * Rt_ArrayAllocate - allocate a new item in the array and return it
 * @a: the array
 *
 * Returns: Pointer to item
 */
static inline void *
Rt_ArrayAllocate(struct NeArray *a)
{
	void *ptr = NULL;

	if (a->count == a->size)
		if (!Rt_ResizeArray(a, _Rt_CalcGrowSize(a->size, a->elemSize, a->size + RT_DEF_INC)))
			return NULL;

	ptr = a->data + a->elemSize * a->count++;
	Sys_ZeroMemory(ptr, a->elemSize);

	return ptr;
}

// Actual insert function
static inline bool __miwa_array_insert(struct NeArray *, const void *, size_t, bool);

/**
 * Rt_ArrayInsert - insert an item in the array
 * @a: the array
 * @item: item to add
 * @pos: position to insert the item at
 *
 * &rt_array.elemSize number of bytes will be copied from item to the @pos
 * location in the array. If &rt_array.count is equal to &rt_array.size the
 * array will grow.
 *
 * Returns: OK on success
 */
static inline bool
Rt_ArrayInsert(struct NeArray *a, const void *item, size_t pos)
{
	return __miwa_array_insert(a, item, pos, true);
}

/**
 * Rt_ArrayInsertPtr - insert a pointer in the array
 * @a: the array
 * @item: item to add
 * @pos: position to insert the item at
 *
 * Convenience function for arrays of pointers. Behaves exactly like
 * rt_array_insert()
 *
 * Returns: OK on success
 */
static inline bool
Rt_ArrayInsertPtr(struct NeArray *a, const void *item, size_t pos)
{
	return __miwa_array_insert(a, &item, pos, true);
}

/**
 * Rt_ArrayFastInsert - insert an item in the array
 * @a: the array
 * @item: item to add
 * @pos: position to insert the item at
 *
 * &rt_array.elemSize number of bytes will be copied from item to the @pos
 * location in the array. The item at @pos will be moved to the end of the
 * array. If &rt_array.count is equal to &rt_array.size the
 * array will grow.
 *
 * Returns: OK on success
 */
static inline bool
Rt_ArrayFastInsert(struct NeArray *a, const void *item, size_t pos)
{
	return __miwa_array_insert(a, item, pos, false);
}

/**
 * Rt_ArrayFastInsertPtr - insert a pointer in the array
 * @a: the array
 * @item: item to add
 * @pos: position to insert the item at
 *
 * Convenience function for arrays of pointers. Behaves exactly like
 * rt_array_fast_insert()
 *
 * Returns: OK on success
 */
static inline bool
Rt_ArrayFastInsertPtr(struct NeArray *a, const void *item, size_t pos)
{
	return __miwa_array_insert(a, &item, pos, false);
}

static inline bool
Rt_ArrayRemove(struct NeArray *a, size_t index)
{
	size_t i = 0;

	if (!a->count || index > a->count - 1)
		return false;

	--a->count;

	for (i = index + 1; i <= a->count; ++i)
		memcpy(a->data + a->elemSize * (i - 1), a->data + a->elemSize * i, a->elemSize);

	return true;
}

static inline void *
Rt_ArrayGet(const struct NeArray *a, size_t id)
{
	if (id > a->size)
		return NULL;

	return a->data + a->elemSize * id;
}

static inline void *
Rt_ArrayGetPtr(const struct NeArray *a, size_t id)
{
	if (id > a->size)
		return NULL;

	return *(void **)Rt_ArrayGet(a, id);
}

static inline void *
Rt_ArrayFind(const struct NeArray *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t i;

	for (i = 0; i < a->count; ++i)
		if (!cmpFunc(a->data + a->elemSize * i, data))
			return a->data + a->elemSize * i;

	return NULL;
}

static inline size_t
Rt_ArrayFindId(const struct NeArray *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t i;

	for (i = 0; i < a->count; ++i)
		if (!cmpFunc(a->data + a->elemSize * i, data))
			return i;

	return RT_NOT_FOUND;
}

static inline size_t
Rt_PtrArrayFindId(const struct NeArray *a, const void *ptr)
{
	size_t i;

	for (i = 0; i < a->count; ++i)
		if (Rt_ArrayGetPtr(a, i) == ptr)
			return i;

	return RT_NOT_FOUND;
}

static inline void *
Rt_ArrayBSearch(const struct NeArray *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t start = 0, end = a->count - 1, mid;
	int32_t c;
	void *elem = 0;

	if (!a->count)
		return NULL;

	while (start <= end && start < a->count && end < a->count) {
		mid = start + (end - start) / 2;

		elem = a->data + a->elemSize * mid;

		c = cmpFunc(elem, data);
		if (!c)
			return elem;
		else if (c < 0)
			start = mid + 1;
		else
			end = mid - 1;
	}

	return NULL;
}

static inline size_t
Rt_ArrayBSearchId(const struct NeArray *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t start = 0, end = a->count - 1, mid;
	int32_t c;
	void *elem = 0;

	if (!a->count)
		return RT_NOT_FOUND;

	while (start <= end && start < a->count && end < a->count) {
		mid = start + (end - start) / 2;

		elem = a->data + a->elemSize * mid;

		c = cmpFunc(elem, data);
		if (!c)
			return mid;
		else if (c < 0)
			start = mid + 1;
		else
			end = mid - 1;
	}

	return RT_NOT_FOUND;
}

static inline void
Rt_ArraySort(struct NeArray *a, RtSortFunc sortFunc)
{
	qsort(a->data, a->count, a->elemSize, sortFunc);
}

static inline bool
Rt_ArrayReverse(struct NeArray *a)
{
	void *tmp = NULL;
	uint64_t s = 0, e = 0;
	void *start = 0, *end = 0;

	if (!a->count)
		return false;

	tmp = Sys_Alloc(1, a->elemSize, MH_Transient);
	if (!tmp)
		return false;

	s = 0;
	e = a->count - 1;

	while (s < e) {
		start = a->data + a->elemSize * s++;
		end = a->data + a->elemSize * e--;

		memcpy(tmp, start, a->elemSize);
		memcpy(start, end, a->elemSize);
		memcpy(end, tmp, a->elemSize);
	}

	return true;
}

static inline size_t
Rt_ArrayUpperBound(const struct NeArray *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t low = 0, mid = 0;
	size_t high = a->count;

	while (low < high) {
		mid = (low + high) / 2;

		if (cmpFunc(a->data + a->elemSize * mid, data) < 0)
			high = mid;
		else
			low = mid + 1;
	}

	return low;
}

static inline size_t
Rt_ArrayLowerBound(const struct NeArray *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t low = 0, mid = 0;
	size_t high = a->count;

	while (low < high) {
		mid = (low + high) / 2;

		if (cmpFunc(a->data + a->elemSize * mid, data) >= 0)
			high = mid;
		else
			low = mid + 1;
	}

	return low;
}

#define Rt_FillArray(a) (a)->count = (a)->size
#define Rt_ZeroArray(a) memset((a)->data, 0x0, (a)->elemSize * (a)->count); a->count = 0

static inline void
Rt_ClearArray(struct NeArray *a, bool freeMemory)
{
	a->count = 0;

	if (!freeMemory || !a->data)
		return;

	a->size = 0;

	if (a->align > 1)
#ifndef SYS_PLATFORM_WINDOWS
		free(a->data);
#else
		_aligned_free(a->data);
#endif
	else
		Sys_Free(a->data);

	a->data = NULL;
}

static inline void
Rt_TermArray(struct NeArray *a)
{
	if (!a)
		return;

	Rt_ClearArray(a, true);
}

#ifdef __cplusplus
#define Rt_ArrayForEach(var, a, type)					\
	for (size_t miwa_rtafei = 0;						\
		(miwa_rtafei < (a)->count) &&					\
			(var = (type)Rt_ArrayGet(a, miwa_rtafei));	\
		++miwa_rtafei)

#define Rt_ArrayForEachPtr(var, a, type)					\
	for (size_t miwa_rtafei = 0;							\
		(miwa_rtafei < (a)->count) &&						\
			(var = (type)Rt_ArrayGetPtr(a, miwa_rtafei));	\
		++miwa_rtafei)
#else
#define Rt_ArrayForEach(var, a)						\
	for (size_t miwa_rtafei = 0;					\
		(miwa_rtafei < (a)->count) &&				\
			(var = Rt_ArrayGet(a, miwa_rtafei));	\
		++miwa_rtafei)

#define Rt_ArrayForEachPtr(var, a)					\
	for (size_t miwa_rtafei = 0;					\
		(miwa_rtafei < (a)->count) &&				\
			(var = Rt_ArrayGetPtr(a, miwa_rtafei));	\
		++miwa_rtafei)
#endif

// You are not supposed to call this function directly;
// Use the wrappers defined above instead.
static inline bool
__miwa_array_insert(struct NeArray *a, const void *data, size_t pos, bool ordered)
{
	size_t i = 0;

	if (a->count == a->size)
		if (Rt_ResizeArray(a, _Rt_CalcGrowSize(a->size,
				a->elemSize, a->size + RT_DEF_INC)))
			return false;

	if (!ordered) {
		memcpy(a->data + a->elemSize * a->count, Rt_ArrayGet(a, pos), a->elemSize);
	} else {
		for (i = a->count; i > pos; --i)
			memcpy(a->data + a->elemSize * i, a->data + a->elemSize * (i - 1), a->elemSize);
	}

	memcpy(a->data + a->elemSize * pos, data, a->elemSize);
	++a->count;

	return true;
}

#ifdef __cplusplus
}
#endif

#endif /* _NE_RUNTIME_ARRAY_H_ */

/* NekoEngine
 *
 * Array.h
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
