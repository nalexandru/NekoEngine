#ifndef _NE_RUNTIME_QUEUE_H_
#define _NE_RUNTIME_QUEUE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <System/System.h>
#include <System/Memory.h>
#include <Runtime/RtDefs.h>

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

	if ((q->data = Sys_Alloc(q->size, q->elemSize, q->heap)) == NULL)
		return false;

	return true;
}

#define Rt_InitPtrQueue(a, size) Rt_InitQueue(a, size, sizeof(void *))

static inline bool
Rt_CloneQueue(struct NeQueue *dst, const struct NeQueue *src, enum NeMemoryHeap heap)
{
	if (!dst || !src)
		return false;

	void *data = Sys_Alloc(src->size, src->elemSize, heap);
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
	uint8_t *tmp = 0;

	if (q->size == size)
		return true;

	tmp = q->data;
	if ((q->data = Sys_ReAlloc(q->data, size, q->elemSize, q->heap)) == NULL) {
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
	void *ptr = 0;

	if (!q->count)
		return NULL;

	--q->count;

	if (q->start == q->size)
		q->start = 0;

	ptr = q->data + q->elemSize * q->start++;

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

#endif /* _NE_RUNTIME_QUEUE_H_ */
