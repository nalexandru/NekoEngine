#include <Engine/Job.h>
#include <Engine/Event.h>
#include <Runtime/Array.h>
#include <System/System.h>
#include <System/Memory.h>
#include <System/AtomicLock.h>

struct Event
{
	uint64_t event;
	uint64_t timestamp;
	void *args;
};

struct EventHandler
{
	EventHandlerProc proc;
	uint64_t timestamp;
	void *user;
};

struct EventHandlerInfo
{
	uint64_t event;
	struct Array handlers;
};

struct ProcessEventArgs
{
	struct EventHandler handler;
	void *args;
};

static struct Array _handlers, _queue[2], *_currentQueue;
static int _currentQueueId;
static struct AtomicLock _queueLock, _handlerLock;

static int32_t _Sort(const struct EventHandlerInfo *a, const struct EventHandlerInfo *b);
static int32_t _Comp(const struct EventHandlerInfo *m, const uint64_t *Event);
static void _ProcessEvent(int worker, struct ProcessEventArgs *args);

void
E_Broadcast(const wchar_t *event, void *args)
{
	if (!_currentQueue)
		return;

	struct Event evt;
	evt.event = Rt_HashStringW(event);
	evt.args = args;
	evt.timestamp = Sys_Time();

	Sys_AtomicLockWrite(&_queueLock);
	Rt_ArrayAdd(_currentQueue, &evt);
	Sys_AtomicUnlockWrite(&_queueLock);
}

uint64_t
E_RegisterHandler(const wchar_t *event, EventHandlerProc handlerProc, void *user)
{
	bool sort = false;
	struct EventHandler *handler = NULL;
	struct EventHandlerInfo *info = NULL;
	uint64_t hash = Rt_HashStringW(event);
	uint32_t idx = 0, handlerIdx = 0;

	Sys_AtomicLockWrite(&_handlerLock);

	handlerIdx = (uint32_t)Rt_ArrayBSearchId(&_handlers, &hash, (RtCmpFunc)_Comp);
	info = Rt_ArrayGet(&_handlers, handlerIdx);
	if (!info) {
		info = Rt_ArrayAllocate(&_handlers);
		info->event = hash;
		Rt_InitArray(&info->handlers, 5, sizeof(struct EventHandler), MH_System);
		sort = true;
	}

	handler = Rt_ArrayAllocate(&info->handlers);
	handler->proc = handlerProc;
	handler->user = user;
	handler->timestamp = Sys_Time();

	idx = (uint32_t)info->handlers.count - 1;

	if (sort) {
		Rt_ArraySort(&_handlers, (RtSortFunc)_Sort);
		handlerIdx = (uint32_t)Rt_ArrayBSearchId(&_handlers, &event, (RtCmpFunc)_Comp);
	}

	Sys_AtomicUnlockWrite(&_handlerLock);

	return (uint64_t)idx | (uint64_t)handlerIdx << 32;
}

void
E_UnregisterHandler(uint64_t handler)
{
	struct EventHandlerInfo *info = NULL;
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
	if (!Rt_InitArray(&_handlers, 10, sizeof(struct EventHandlerInfo), MH_System))
		return false;

	if (!Rt_InitArray(&_queue[0], 10, sizeof(struct Event), MH_System))
		return false;

	if (!Rt_InitArray(&_queue[1], 10, sizeof(struct Event), MH_System))
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
	struct EventHandlerInfo *info;

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
	struct Event *evt;
	struct EventHandlerInfo *info;
	struct ProcessEventArgs *args;
	struct Array *queue = _currentQueue;

	Sys_AtomicLockWrite(&_queueLock);
	_currentQueueId = !_currentQueueId;
	_currentQueue = &_queue[_currentQueueId];
	Sys_AtomicUnlockWrite(&_queueLock);

	Sys_AtomicLockRead(&_handlerLock);

	for (i = 0; i < queue->count; ++i) {
		evt = Rt_ArrayGet(queue, i);
		info = Rt_ArrayBSearch(&_handlers, &evt->event, (RtCmpFunc)_Comp);
		if (!info)
			continue;

		for (j = 0; j < info->handlers.count; ++j) {
			struct EventHandler *handler = Rt_ArrayGet(&info->handlers, j);

			if (handler->timestamp > evt->timestamp)
				continue;

			args = Sys_Alloc(sizeof(*args), 1, MH_Frame);
			args->handler = *handler;
			args->args = evt->args;
			E_ExecuteJob((JobProc)_ProcessEvent, args, NULL);
		}
	}

	Rt_ClearArray(queue, false);

	Sys_AtomicUnlockRead(&_handlerLock);

	//
}

static int32_t
_Sort(const struct EventHandlerInfo *a, const struct EventHandlerInfo *b)
{
	return a->event > b->event ? 1 : (a->event < b->event ? -1 : 0);
}

static int32_t
_Comp(const struct EventHandlerInfo *m, const uint64_t *event)
{
	return m->event > *event ? 1 : (m->event < *event ? -1 : 0);
}

void
_ProcessEvent(int worker, struct ProcessEventArgs *args)
{
	args->handler.proc(args->handler.user, args->args);
}
