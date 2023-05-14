#ifndef NE_RUNTIME_QUEUE_H
#define NE_RUNTIME_QUEUE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <System/System.h>
#include <System/Memory.h>
#include <Runtime/RtDefs.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NeQueue
{
	uint8_t *data;
	size_t size;
	size_t start;
	size_t end;
	size_t count;
	size_t elemSize;
	enum NeMemoryHeap heap;
};

static inline bool
Rt_InitQueue(struct NeQueue *q, size_t size, size_t elemSize, enum NeMemoryHeap heap)
{
	if (!q)
		return false;

	q->count = q->start = q->end = 0;
	q->elemSize = elemSize;
	q->size = size;
	q->heap = heap;

	if ((q->data = (uint8_t *)Sys_Alloc(q->size, q->elemSize, q->heap)) == NULL)
		return false;

	return true;
}

#define Rt_InitPtrQueue(a, size, heap) Rt_InitQueue(a, size, sizeof(void *), heap)

static inline bool
Rt_CloneQueue(struct NeQueue *dst, const struct NeQueue *src, enum NeMemoryHeap heap)
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

static inline bool
Rt_ResizeQueue(struct NeQueue *q, size_t size)
{
	if (q->size == size)
		return true;

	uint8_t *tmp = q->data;
	if ((q->data = (uint8_t *)Sys_ReAlloc(q->data, size, q->elemSize, q->heap)) == NULL) {
		q->data = tmp;
		return false;
	}

	q->size = size;

	if (q->size < q->count)
		q->count = q->size;

	return true;
}

static inline bool
Rt_GrowQueue(struct NeQueue *q)
{
	return Rt_ResizeQueue(q, _Rt_CalcGrowSize(q->size, q->elemSize, q->size + RT_DEF_INC));
}

static inline bool
Rt_QueuePush(struct NeQueue *q, void *data)
{
	if (q->count == q->size)
		if (!Rt_GrowQueue(q))
			return false;

	if (q->end == q->size)
		q->end = 0;

	memcpy(q->data + q->elemSize * q->end++, data, q->elemSize);
	++q->count;

	return true;
}

static inline bool
Rt_QueuePushPtr(struct NeQueue *q, void *data)
{
	return Rt_QueuePush(q, &data);
}

static inline void *
Rt_QueuePeek(const struct NeQueue *q)
{
	if (!q->count)
		return NULL;

	return q->data + q->elemSize * q->start;
}

static inline void *
Rt_QueuePeekPtr(const struct NeQueue *q)
{
	return *(void **)Rt_QueuePeek(q);
}

static inline void *
Rt_QueuePop(struct NeQueue *q)
{
	if (!q->count)
		return NULL;

	--q->count;

	if (q->start == q->size)
		q->start = 0;

	void *ptr = q->data + q->elemSize * q->start++;

	if (!q->count)
		q->start = q->end = 0;

	return ptr;
}

static inline void *
Rt_QueuePopPtr(struct NeQueue *q)
{
	return *(void **)Rt_QueuePop(q);
}

#define Rt_FillQueue(q) q->count = q->size

static inline void
Rt_ClearQueue(struct NeQueue *q, bool freeMemory)
{
	q->count = 0;

	if (!freeMemory)
		return;

	q->size = 0;
	Sys_Free(q->data);
	q->data = NULL;
}


static inline void
Rt_TermQueue(struct NeQueue *q)
{
	Rt_ClearQueue(q, true);
}

#ifdef __cplusplus
}
#endif

#endif /* NE_RUNTIME_QUEUE_H */

/* NekoEngine
 *
 * Queue.h
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
