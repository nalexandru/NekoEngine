#ifndef NE_RUNTIME_TSQUEUE_H
#define NE_RUNTIME_TSQUEUE_H

#include <Runtime/Queue.h>
#include <System/Thread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NeTSQueue
{
	struct NeQueue q;
	NeFutex ftx;
};

static inline bool
Rt_InitTSQueue(struct NeTSQueue *q, size_t size, size_t elemSize, enum NeMemoryHeap heap)
{
	if (!Sys_InitFutex(&q->ftx))
		return false;

	Sys_LockFutex(q->ftx);
	const bool rc = Rt_InitQueue(&q->q, size, elemSize, heap);
	Sys_UnlockFutex(q->ftx);

	return rc;
}

#define Rt_InitPtrTSQueue(a, size, heap) Rt_InitTSQueue(a, size, sizeof(void *), heap)

static inline bool
Rt_CloneTSQueue(struct NeTSQueue *dst, const struct NeTSQueue *src, enum NeMemoryHeap heap)
{
	if (!Sys_InitFutex(&dst->ftx))
		return false;

	Sys_LockFutex(dst->ftx);
	Sys_LockFutex(src->ftx);
	const bool rc = Rt_CloneQueue(&dst->q, &src->q, heap);
	Sys_UnlockFutex(src->ftx);
	Sys_UnlockFutex(dst->ftx);

	return rc;
}

static inline bool
Rt_ResizeTSQueue(struct NeTSQueue *q, size_t size)
{
	Sys_LockFutex(q->ftx);
	const bool rc = Rt_ResizeQueue(&q->q, size);
	Sys_UnlockFutex(q->ftx);

	return rc;
}

static inline bool
Rt_GrowTSQueue(struct NeTSQueue *q)
{
	Sys_LockFutex(q->ftx);
	const bool rc = Rt_GrowQueue(&q->q);
	Sys_UnlockFutex(q->ftx);

	return rc;
}

static inline bool
Rt_TSQueuePush(struct NeTSQueue *q, void *data)
{
	Sys_LockFutex(q->ftx);
	const bool rc = Rt_QueuePush(&q->q, data);
	Sys_UnlockFutex(q->ftx);

	return rc;
}

static inline bool
Rt_TSQueuePushPtr(struct NeTSQueue *q, void *data)
{
	Sys_LockFutex(q->ftx);
	const bool rc = Rt_QueuePushPtr(&q->q, data);
	Sys_UnlockFutex(q->ftx);

	return rc;
}

static inline void *
Rt_TSQueuePeek(const struct NeTSQueue *q)
{
	Sys_LockFutex(q->ftx);
	void *r = Rt_QueuePeek(&q->q);
	Sys_UnlockFutex(q->ftx);

	return r;
}

static inline void *
Rt_TSQueuePeekPtr(const struct NeTSQueue *q)
{
	Sys_LockFutex(q->ftx);
	void *r = Rt_QueuePeekPtr(&q->q);
	Sys_UnlockFutex(q->ftx);

	return r;
}

static inline void *
Rt_TSQueuePop(struct NeTSQueue *q)
{
	Sys_LockFutex(q->ftx);
	void *r = Rt_QueuePop(&q->q);
	Sys_UnlockFutex(q->ftx);

	return r;
}

static inline void *
Rt_TSQueuePopPtr(struct NeTSQueue *q)
{
	Sys_LockFutex(q->ftx);
	void *r = Rt_QueuePopPtr(&q->q);
	Sys_UnlockFutex(q->ftx);

	return r;
}

#define Rt_FillTSQueue(q) q->count = q->size

static inline void
Rt_ClearTSQueue(struct NeTSQueue *q, bool freeMemory)
{
	Sys_LockFutex(q->ftx);
	Rt_ClearQueue(&q->q, freeMemory);
	Sys_UnlockFutex(q->ftx);
}


static inline void
Rt_TermTSQueue(struct NeTSQueue *q)
{
	Sys_LockFutex(q->ftx);
	Rt_TermQueue(&q->q);
	Sys_UnlockFutex(q->ftx);

	Sys_TermFutex(q->ftx);
}

static inline bool
Rt_LockTSQueue(struct NeTSQueue *q)
{
	return Sys_LockFutex(q->ftx);
}

static inline bool
Rt_UnlockTSQueue(struct NeTSQueue *q)
{
	return Sys_UnlockFutex(q->ftx);
}

#ifdef __cplusplus
}
#endif

#endif /* NE_RUNTIME_TSQUEUE_H */

/* NekoEngine
 *
 * TSQueue.h
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
