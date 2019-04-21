/* Miwa Portable Runtime
 *
 * queue.c
 * Author: Alexandru Naiman
 *
 * Queue
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

#include <system/compat.h>
#include <runtime/runtime.h>

#include "rt_internal.h"

int
rt_queue_init(rt_queue *q,
	size_t size,
	size_t elem_size)
{
	if (!q)
		return SYS_INVALID_ARGS;

	q->count = q->start = q->end = 0;
	q->elem_size = elem_size;
	q->size = size;

	if ((q->data = reallocarray(NULL, q->size, q->elem_size)) == NULL)
		return SYS_MEMORY;
	
	return SYS_OK;
}

rt_queue *
rt_queue_clone(rt_queue *q)
{
	return NULL;
}

int
rt_queue_push(rt_queue *q,
	void *data)
{
	int ret = SYS_OK;

	if (q->count == q->size)
		if ((ret = rt_queue_grow(q)) != SYS_OK)
			return ret;

	if (q->end == q->size)
		q->end = 0;

	memcpy(q->data + q->elem_size * q->end++, data, q->elem_size);
	++q->count;

	return SYS_OK;
}

int
rt_queue_push_ptr(rt_queue *q,
	void *data)
{
	return rt_queue_push(q, &data);
}

void *
rt_queue_peek(rt_queue *q)
{
	if (!q->count)
		return NULL;

	return q->data + q->elem_size * q->start;
}

void *
rt_queue_peek_ptr(rt_queue *q)
{
	return *(void **)rt_queue_peek(q);
}

void *
rt_queue_pop(rt_queue *q)
{
	void *ptr = 0;

	if (!q->count)
		return NULL;

	--q->count;

	if (q->start == q->size)
		q->start = 0;

	ptr = q->data + q->elem_size * q->start++;

	if (!q->count)
		q->start = q->end = 0;

	return ptr;
}

void *
rt_queue_pop_ptr(rt_queue *q)
{
	return *(void **)rt_queue_pop(q);
}

int
rt_queue_grow(rt_queue *q)
{
	return rt_queue_resize(q,
		_rt_calc_grow_size(q->size, q->elem_size, q->size + RT_DEF_INC));
}

int
rt_queue_resize(rt_queue *q,
	size_t size)
{
	uint8_t *tmp = 0;

	if (q->size == size)
		return SYS_OK;

	tmp = q->data;
	if ((q->data = reallocarray(q->data, size, q->elem_size)) == NULL) {
		q->data = tmp;
		return SYS_MEMORY;
	}

	q->size = size;

	if (q->size < q->count)
		q->count = q->size;

	return SYS_OK;
}

void
rt_queue_clear(rt_queue *q,
	bool free_memory)
{
	q->count = 0;

	if (!free_memory)
		return;

	q->size = 0;
	free(q->data);
	q->data = NULL;
}

void
rt_queue_release(rt_queue *q)
{
	rt_queue_clear(q, true);
}

