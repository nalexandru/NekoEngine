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

static struct NeArray _handlers, _queue[2], *_currentQueue;
static int _currentQueueId;
static struct NeAtomicLock _queueLock, _handlerLock;

static void _ProcessEvent(int worker, struct NeProcessEventArgs *args);

void
E_Broadcast(const char *event, void *args)
{
	if (!_currentQueue)
		return;

	struct NeEvent evt;
	evt.event = Rt_HashString(event);
	evt.args = args;
	evt.timestamp = Sys_Time();

	Sys_AtomicLockWrite(&_queueLock);
	Rt_ArrayAdd(_currentQueue, &evt);
	Sys_AtomicUnlockWrite(&_queueLock);
}

uint64_t
E_RegisterHandler(const char *event, NeEventHandlerProc handlerProc, void *user)
{
	bool sort = false;
	struct NeEventHandler *handler = NULL;
	struct NeEventHandlerInfo *info = NULL;
	uint64_t hash = Rt_HashString(event);
	uint32_t idx = 0, handlerIdx = 0;

	Sys_AtomicLockWrite(&_handlerLock);

	handlerIdx = (uint32_t)Rt_ArrayBSearchId(&_handlers, &hash, Rt_U64CmpFunc);
	info = Rt_ArrayGet(&_handlers, handlerIdx);
	if (!info) {
		info = Rt_ArrayAllocate(&_handlers);
		info->event = hash;
		Rt_InitArray(&info->handlers, 5, sizeof(struct NeEventHandler), MH_System);
		sort = true;
	}

	handler = Rt_ArrayAllocate(&info->handlers);
	handler->proc = handlerProc;
	handler->user = user;
	handler->timestamp = Sys_Time();

	idx = (uint32_t)info->handlers.count - 1;

	if (sort) {
		Rt_ArraySort(&_handlers, Rt_U64CmpFunc);
		handlerIdx = (uint32_t)Rt_ArrayBSearchId(&_handlers, &event, Rt_U64CmpFunc);
	}

	Sys_AtomicUnlockWrite(&_handlerLock);

	return (uint64_t)idx | (uint64_t)handlerIdx << 32;
}

void
E_UnregisterHandler(uint64_t handler)
{
	struct NeEventHandlerInfo *info = NULL;
	uint32_t handlerIdx = (uint32_t)((handler & (uint64_t)0xFFFFFFFF00000000) >> 32);
	uint32_t infoIdx = (uint32_t)(handler & (uint64_t)0x00000000FFFFFFFF);

	Sys_AtomicLockWrite(&_handlerLock);

	info = Rt_ArrayGet(&_handlers, infoIdx);
	if (info) {
		Rt_ArrayRemove(&info->handlers, handlerIdx);
		if (!info->handlers.count)
			Rt_ArrayRemove(&_handlers, handlerIdx);
	}

	Sys_AtomicUnlockWrite(&_handlerLock);
}

bool
E_InitEventSystem(void)
{
	if (!Rt_InitArray(&_handlers, 10, sizeof(struct NeEventHandlerInfo), MH_System))
		return false;

	if (!Rt_InitArray(&_queue[0], 10, sizeof(struct NeEvent), MH_System))
		return false;

	if (!Rt_InitArray(&_queue[1], 10, sizeof(struct NeEvent), MH_System))
		return false;

	_currentQueue = &_queue[0];
	_currentQueueId = 0;

	Sys_InitAtomicLock(&_queueLock);
	Sys_InitAtomicLock(&_handlerLock);

	return true;
}

void
E_TermEventSystem(void)
{
	size_t i = 0;
	struct NeEventHandlerInfo *info;

	for (i = 0; i < _handlers.count; ++i) {
		info = Rt_ArrayGet(&_handlers, i);
		Rt_TermArray(&info->handlers);
	}

	Rt_TermArray(&_queue[0]);
	Rt_TermArray(&_queue[1]);
	Rt_TermArray(&_handlers);
}

void
E_ProcessEvents(void)
{
	size_t i = 0, j = 0;
	struct NeEvent *evt;
	struct NeEventHandlerInfo *info;
	struct NeProcessEventArgs *args;
	struct NeArray *queue = _currentQueue;

	Sys_AtomicLockWrite(&_queueLock);
	_currentQueueId = !_currentQueueId;
	_currentQueue = &_queue[_currentQueueId];
	Sys_AtomicUnlockWrite(&_queueLock);

	Sys_AtomicLockRead(&_handlerLock);

	for (i = 0; i < queue->count; ++i) {
		evt = Rt_ArrayGet(queue, i);
		info = Rt_ArrayBSearch(&_handlers, &evt->event, Rt_U64CmpFunc);
		if (!info)
			continue;

		for (j = 0; j < info->handlers.count; ++j) {
			struct NeEventHandler *handler = Rt_ArrayGet(&info->handlers, j);

			if (handler->timestamp > evt->timestamp)
				continue;

			args = Sys_Alloc(sizeof(*args), 1, MH_Frame);
			args->handler = *handler;
			args->args = evt->args;
			E_ExecuteJob((NeJobProc)_ProcessEvent, args, NULL, NULL);
		}
	}

	Rt_ClearArray(queue, false);

	Sys_AtomicUnlockRead(&_handlerLock);

	//
}

void
_ProcessEvent(int worker, struct NeProcessEventArgs *args)
{
	if (!args->handler.proc)
		return;

	args->handler.proc(args->handler.user, args->args);
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
