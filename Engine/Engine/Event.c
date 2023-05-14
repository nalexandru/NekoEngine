#include <Engine/Job.h>
#include <Engine/Event.h>
#include <Runtime/Array.h>
#include <System/System.h>
#include <System/Memory.h>
#include <System/AtomicLock.h>

struct NeEvent
{
	uint64_t event;
	uint64_t timestamp;
	void *args;
};

struct NeEventHandler
{
	NeEventHandlerProc proc;
	uint64_t timestamp;
	void *user;
};

struct NeEventHandlerInfo
{
	uint64_t event;
	struct NeArray handlers;
};

struct NeProcessEventArgs
{
	struct NeEventHandler handler;
	void *args;
};

static struct NeArray f_handlers, f_queue[2], *f_currentQueue;
static int f_currentQueueId;
static struct NeAtomicLock f_queueLock, f_handlerLock;
static NE_ATOMIC_UINT f_jobCount;

static void ProcessEvent(int worker, struct NeProcessEventArgs *args);
static void JobCompleted(uint64_t id, void *args);

void
E_Broadcast(const char *event, void *args)
{
	if (!f_currentQueue)
		return;

	struct NeEvent evt;
	evt.event = Rt_HashString(event);
	evt.args = args;
	evt.timestamp = Sys_Time();

	Sys_AtomicLockWrite(&f_queueLock);
	Rt_ArrayAdd(f_currentQueue, &evt);
	Sys_AtomicUnlockWrite(&f_queueLock);
}

uint64_t
E_RegisterHandler(const char *event, NeEventHandlerProc handlerProc, void *user)
{
	bool sort = false;
	struct NeEventHandler *handler = NULL;
	struct NeEventHandlerInfo *info = NULL;
	uint64_t hash = Rt_HashString(event);

	Sys_AtomicLockWrite(&f_handlerLock);

	uint32_t handlerIdx = (uint32_t)Rt_ArrayBSearchId(&f_handlers, &hash, Rt_U64CmpFunc);
	info = Rt_ArrayGet(&f_handlers, handlerIdx);
	if (!info) {
		info = Rt_ArrayAllocate(&f_handlers);
		info->event = hash;
		Rt_InitArray(&info->handlers, 5, sizeof(struct NeEventHandler), MH_System);
		sort = true;
	}

	handler = Rt_ArrayAllocate(&info->handlers);
	handler->proc = handlerProc;
	handler->user = user;
	handler->timestamp = Sys_Time();

	const uint32_t idx = (uint32_t)info->handlers.count - 1;

	if (sort) {
		Rt_ArraySort(&f_handlers, Rt_U64CmpFunc);
		handlerIdx = (uint32_t)Rt_ArrayBSearchId(&f_handlers, &event, Rt_U64CmpFunc);
	}

	Sys_AtomicUnlockWrite(&f_handlerLock);

	return (uint64_t)idx | (uint64_t)handlerIdx << 32;
}

void
E_UnregisterHandler(uint64_t handler)
{
	struct NeEventHandlerInfo *info = NULL;
	uint32_t handlerIdx = (uint32_t)((handler & (uint64_t)0xFFFFFFFF00000000) >> 32);
	uint32_t infoIdx = (uint32_t)(handler & (uint64_t)0x00000000FFFFFFFF);

	Sys_AtomicLockWrite(&f_handlerLock);

	info = Rt_ArrayGet(&f_handlers, infoIdx);
	if (info) {
		Rt_ArrayRemove(&info->handlers, handlerIdx);
		if (!info->handlers.count)
			Rt_ArrayRemove(&f_handlers, handlerIdx);
	}

	Sys_AtomicUnlockWrite(&f_handlerLock);
}

bool
E_InitEventSystem(void)
{
	if (!Rt_InitArray(&f_handlers, 10, sizeof(struct NeEventHandlerInfo), MH_System))
		return false;

	if (!Rt_InitArray(&f_queue[0], 10, sizeof(struct NeEvent), MH_System))
		return false;

	if (!Rt_InitArray(&f_queue[1], 10, sizeof(struct NeEvent), MH_System))
		return false;

	f_currentQueue = &f_queue[0];
	f_currentQueueId = 0;

	Sys_InitAtomicLock(&f_queueLock);
	Sys_InitAtomicLock(&f_handlerLock);

	return true;
}

void
E_TermEventSystem(void)
{
	struct NeEventHandlerInfo *info;
	Rt_ArrayForEach(info, &f_handlers)
		Rt_TermArray(&info->handlers);

	Rt_TermArray(&f_queue[0]);
	Rt_TermArray(&f_queue[1]);
	Rt_TermArray(&f_handlers);
}

void
E_ProcessEvents(void)
{
	struct NeEvent *evt;
	struct NeEventHandlerInfo *info;
	struct NeProcessEventArgs *args;
	struct NeArray *queue = f_currentQueue;

	Sys_AtomicLockWrite(&f_queueLock);
	f_currentQueueId = !f_currentQueueId;
	f_currentQueue = &f_queue[f_currentQueueId];
	Sys_AtomicUnlockWrite(&f_queueLock);

	Sys_AtomicLockRead(&f_handlerLock);

	atomic_store(&f_jobCount, 0);

	for (size_t i = 0; i < queue->count; ++i) {
		evt = Rt_ArrayGet(queue, i);
		info = Rt_ArrayBSearch(&f_handlers, &evt->event, Rt_U64CmpFunc);
		if (!info)
			continue;

		for (size_t j = 0; j < info->handlers.count; ++j) {
			struct NeEventHandler *handler = Rt_ArrayGet(&info->handlers, j);

			if (handler->timestamp > evt->timestamp)
				continue;

			args = Sys_Alloc(sizeof(*args), 1, MH_Frame);
			args->handler = *handler;
			args->args = evt->args;

			atomic_fetch_add_explicit(&f_jobCount, 1, memory_order_acquire);
			E_ExecuteJob((NeJobProc)ProcessEvent, args, JobCompleted, NULL);
		}
	}

	Rt_ClearArray(queue, false);

	Sys_AtomicUnlockRead(&f_handlerLock);

	while (atomic_load(&f_jobCount)) ;
}

static void
ProcessEvent(int worker, struct NeProcessEventArgs *args)
{
	if (!args->handler.proc)
		return;

	args->handler.proc(args->handler.user, args->args);
}

static void
JobCompleted(uint64_t id, void *args)
{
	atomic_fetch_sub_explicit(&f_jobCount, 1, memory_order_release);
}

/* NekoEngine
 *
 * Event.c
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
