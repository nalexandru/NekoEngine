/* Miwa Portable Runtime
 *
 * queue.h
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

#ifndef _MIWA_RUNTIME_QUEUE_H_
#define _MIWA_RUNTIME_QUEUE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rt_queue 
{
	uint8_t *data;
	size_t size;
	size_t start;
	size_t end;
	size_t count;
	size_t elem_size;
} rt_queue;

int rt_queue_init(rt_queue *q, size_t size, size_t elem_size);
rt_queue *rt_queue_clone(rt_queue *q);

int rt_queue_push(rt_queue *q, void *i);
int rt_queue_push_ptr(rt_queue *q, void *i);

void *rt_queue_peek(rt_queue *q);
void *rt_queue_peek_ptr(rt_queue *q);

void *rt_queue_pop(rt_queue *q);
void *rt_queue_pop_ptr(rt_queue *q);

int rt_queue_grow(rt_queue *q);
int rt_queue_resize(rt_queue *q, size_t size);

//void rt_queue_fill(rt_queue *q);
#define rt_queue_fill(q) q->count = q->size

void rt_queue_clear(rt_queue *q, bool free_memory);

void rt_queue_release(rt_queue *q);

#ifdef __cplusplus
}
#endif

#endif /* _MIWA_RUNTIME_QUEUE_H_ */

