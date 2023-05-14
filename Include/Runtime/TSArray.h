#ifndef NE_RUNTIME_TSARRAY_H
#define NE_RUNTIME_TSARRAY_H

#include <Runtime/Array.h>
#include <System/Thread.h>

struct NeTSArray
{
	struct NeArray a;
	NeFutex ftx;
};

static inline bool
Rt_InitTSArray(struct NeTSArray *a, size_t size, size_t elemSize, enum NeMemoryHeap heap)
{
	if (!Sys_InitFutex(&a->ftx))
		return false;

	Sys_LockFutex(a->ftx);
	const bool rc = Rt_InitArray(&a->a, size, elemSize, heap);
	Sys_UnlockFutex(a->ftx);

	return rc;
}

static inline bool
Rt_InitAlignedTSArray(struct NeTSArray *a, size_t size, size_t elemSize, size_t alignment, enum NeMemoryHeap heap)
{
	if (!Sys_InitFutex(&a->ftx))
		return false;

	Sys_LockFutex(a->ftx);
	const bool rc = Rt_InitAlignedArray(&a->a, size, elemSize, alignment, heap);
	Sys_UnlockFutex(a->ftx);

	return rc;
}

#define Rt_InitPtrTSArray(a, size, heap) Rt_InitTSArray(a, size, sizeof(void *), heap)

static inline bool
Rt_CloneTSArray(struct NeTSArray *dst, const struct NeTSArray *src, enum NeMemoryHeap heap)
{
	if (!Sys_InitFutex(&dst->ftx))
		return false;

	Sys_LockFutex(dst->ftx);
	Sys_LockFutex(src->ftx);
	const bool rc = Rt_CloneArray(&dst->a, &src->a, heap);
	Sys_UnlockFutex(src->ftx);
	Sys_UnlockFutex(dst->ftx);

	return rc;
}

static inline void *
Rt_ArrayTSFirst(const struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	void *r = Rt_ArrayFirst(&a->a);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline void *
Rt_TSArrayLast(const struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	void *r = Rt_ArrayLast(&a->a);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline size_t
Rt_TSArrayCount(const struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	const size_t r = Rt_ArrayCount(&a->a);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline size_t
Rt_TSArraySize(const struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	const size_t r = Rt_ArraySize(&a->a);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline size_t
Rt_TSArrayByteSize(const struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	const size_t r = Rt_ArrayByteSize(&a->a);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline size_t
Rt_TSArrayUsedByteSize(const struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	const size_t r = Rt_ArrayUsedByteSize(&a->a);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline void *
Rt_TSArrayDataPtr(const struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	void *r = Rt_ArrayDataPtr(&a->a);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline int
Rt_ResizeTSArray(struct NeTSArray *a, size_t size)
{
	Sys_LockFutex(a->ftx);
	const int rc = Rt_ResizeArray(&a->a, size);
	Sys_UnlockFutex(a->ftx);

	return rc;
}

static inline bool
Rt_CopyTSArray(struct NeTSArray *dst, const struct NeTSArray *src)
{
	Sys_LockFutex(dst->ftx);
	Sys_LockFutex(src->ftx);
	const bool rc = Rt_CopyArray(&dst->a, &src->a);
	Sys_UnlockFutex(src->ftx);
	Sys_UnlockFutex(dst->ftx);

	return rc;
}

static inline bool
Rt_TSArrayAdd(struct NeTSArray *a, const void *data)
{
	Sys_LockFutex(a->ftx);
	const bool rc = Rt_ArrayAdd(&a->a, data);
	Sys_UnlockFutex(a->ftx);

	return rc;
}

static inline bool
Rt_TSArrayAddPtr(struct NeTSArray *a, const void *data) { return Rt_TSArrayAdd(a, &data); }

static inline bool
Rt_TSArrayAddArray(struct NeTSArray *dst, const struct NeArray *src)
{
	Sys_LockFutex(dst->ftx);
	const bool rc = Rt_ArrayAddArray(&dst->a, src);
	Sys_UnlockFutex(dst->ftx);

	return rc;
}

static inline bool
Rt_ArrayAddTSArray(struct NeArray *dst, const struct NeTSArray *src)
{
	Sys_LockFutex(src->ftx);
	const bool rc = Rt_ArrayAddArray(dst, &src->a);
	Sys_UnlockFutex(src->ftx);

	return rc;
}

static inline bool
Rt_TSArrayAddTSArray(struct NeTSArray *dst, const struct NeTSArray *src)
{
	Sys_LockFutex(dst->ftx);
	Sys_LockFutex(src->ftx);
	const bool rc = Rt_ArrayAddArray(&dst->a, &src->a);
	Sys_UnlockFutex(src->ftx);
	Sys_UnlockFutex(dst->ftx);

	return rc;
}

static inline void *
Rt_TSArrayAllocate(struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	void *r = Rt_ArrayAllocate(&a->a);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline bool
Rt_TSArrayInsert(struct NeTSArray *a, const void *item, size_t pos)
{
	Sys_LockFutex(a->ftx);
	const bool rc = Rt_ArrayInsert(&a->a, item, pos);
	Sys_UnlockFutex(a->ftx);

	return rc;
}

static inline bool
Rt_TSArrayInsertPtr(struct NeTSArray *a, const void *item, size_t pos)
{
	Sys_LockFutex(a->ftx);
	const bool rc = Rt_ArrayInsertPtr(&a->a, item, pos);
	Sys_UnlockFutex(a->ftx);

	return rc;
}

static inline bool
Rt_TSArrayFastInsert(struct NeTSArray *a, const void *item, size_t pos)
{
	Sys_LockFutex(a->ftx);
	const bool rc = Rt_ArrayFastInsert(&a->a, item, pos);
	Sys_UnlockFutex(a->ftx);

	return rc;
}

static inline bool
Rt_TSArrayFastInsertPtr(struct NeTSArray *a, const void *item, size_t pos)
{
	Sys_LockFutex(a->ftx);
	const bool rc = Rt_ArrayFastInsertPtr(&a->a, item, pos);
	Sys_UnlockFutex(a->ftx);

	return rc;
}

static inline bool
Rt_TSArrayRemove(struct NeTSArray *a, size_t index)
{
	Sys_LockFutex(a->ftx);
	const bool rc = Rt_ArrayRemove(&a->a, index);
	Sys_UnlockFutex(a->ftx);

	return rc;
}

static inline void *
Rt_TSArrayGet(const struct NeTSArray *a, size_t id)
{
	Sys_LockFutex(a->ftx);
	void *r = Rt_ArrayGet(&a->a, id);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline void *
Rt_TSArrayGetPtr(const struct NeTSArray *a, size_t id)
{
	Sys_LockFutex(a->ftx);
	void *r = Rt_ArrayGetPtr(&a->a, id);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline void *
Rt_TSArrayFind(const struct NeTSArray *a, const void *data, RtCmpFunc cmpFunc)
{
	Sys_LockFutex(a->ftx);
	void *r = Rt_ArrayFind(&a->a, data, cmpFunc);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline size_t
Rt_TSArrayFindId(const struct NeTSArray *a, const void *data, RtCmpFunc cmpFunc)
{
	Sys_LockFutex(a->ftx);
	const size_t r = Rt_ArrayFindId(&a->a, data, cmpFunc);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline size_t
Rt_PtrTSArrayFindId(const struct NeTSArray *a, const void *ptr)
{
	Sys_LockFutex(a->ftx);
	const size_t r = Rt_PtrArrayFindId(&a->a, ptr);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline void *
Rt_TSArrayBSearch(const struct NeTSArray *a, const void *data, RtCmpFunc cmpFunc)
{
	Sys_LockFutex(a->ftx);
	void *r = Rt_ArrayBSearch(&a->a, data, cmpFunc);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline size_t
Rt_TSArrayBSearchId(const struct NeTSArray *a, const void *data, RtCmpFunc cmpFunc)
{
	Sys_LockFutex(a->ftx);
	const size_t r = Rt_ArrayBSearchId(&a->a, data, cmpFunc);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline void
Rt_TSArraySort(struct NeTSArray *a, RtSortFunc sortFunc)
{
	Sys_LockFutex(a->ftx);
	Rt_ArraySort(&a->a, sortFunc);
	Sys_UnlockFutex(a->ftx);
}

static inline bool
Rt_TSArrayReverse(struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	const bool rc = Rt_ArrayReverse(&a->a);
	Sys_UnlockFutex(a->ftx);

	return rc;
}

static inline size_t
Rt_TSArrayUpperBound(const struct NeTSArray *a, const void *data, RtCmpFunc cmpFunc)
{
	Sys_LockFutex(a->ftx);
	const size_t r = Rt_ArrayUpperBound(&a->a, data, cmpFunc);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline size_t
Rt_TSArrayLowerBound(const struct NeTSArray *a, const void *data, RtCmpFunc cmpFunc)
{
	Sys_LockFutex(a->ftx);
	const size_t r = Rt_ArrayLowerBound(&a->a, data, cmpFunc);
	Sys_UnlockFutex(a->ftx);

	return r;
}

static inline void
Rt_FillTSArray(struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	Rt_FillArray(&a->a);
	Sys_UnlockFutex(a->ftx);
}

static inline void
Rt_ZeroTSArray(struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	Rt_ZeroArray(&a->a);
	Sys_UnlockFutex(a->ftx);
}

static inline void
Rt_ClearTSArray(struct NeTSArray *a, bool freeMemory)
{
	Sys_LockFutex(a->ftx);
	Rt_ClearArray(&a->a, freeMemory);
	Sys_UnlockFutex(a->ftx);
}

static inline void
Rt_TermTSArray(struct NeTSArray *a)
{
	Sys_LockFutex(a->ftx);
	Rt_TermArray(&a->a);
	Sys_UnlockFutex(a->ftx);

	Sys_TermFutex(a->ftx);
}

static inline bool
Rt_LockTSArray(struct NeTSArray *a)
{
	return Sys_LockFutex(a->ftx);
}

static inline bool
Rt_UnlockTSArray(struct NeTSArray *a)
{
	return Sys_UnlockFutex(a->ftx);
}

#endif /* NE_RUNTIME_TSARRAY_H */

/* NekoEngine
 *
 * TSArray.h
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
