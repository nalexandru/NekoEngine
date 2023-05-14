#include <System/Log.h>
#include <System/Thread.h>
#include <Engine/Event.h>
#include <Script/Script.h>
#include <Runtime/Runtime.h>
#include <Script/Interface.h>

#include "EngineInterface.h"

struct HandlerData
{
	uint64_t handler;
	lua_State *vm;
	int32_t func;
	void *user;
};

static NeMutex _mtx;
static struct NeArray _handlerData;
static struct NeQueue _freeHandlers;
static void _ScriptEventHandler(struct HandlerData *data, void *args);
static void _CleanupHandlerData(void *data, void *args);

SIF_FUNC(Broadcast)
{
	E_Broadcast(luaL_checkstring(vm, 1), NULL);
	return 0;
}

SIF_FUNC(RegisterHandler)
{
	const char *event = luaL_checkstring(vm, 1);
	luaL_checktype(vm, 2, LUA_TFUNCTION);

	Sys_LockMutex(_mtx);

	size_t *free = Rt_QueuePop(&_freeHandlers);
	struct HandlerData *data = free ? Rt_ArrayGet(&_handlerData, *free) : Rt_ArrayAllocate(&_handlerData);

	Sys_UnlockMutex(_mtx);

	data->vm = vm;
	data->user = NULL;

	if (lua_islightuserdata(vm, 3))
		data->user = lua_touserdata(vm, 3);
	else if (!lua_isnil(vm, 3))
		luaL_argerror(vm, 1, "");

	int32_t top = lua_gettop(vm);
	lua_settop(vm, 2);
	data->func = luaL_ref(vm, LUA_REGISTRYINDEX);
	lua_settop(vm, top);

	data->handler = E_RegisterHandler(event, (NeEventHandlerProc)_ScriptEventHandler, &data);

	lua_pushinteger(vm, data->handler);
	return 1;
}

SIF_FUNC(UnregisterHandler)
{
	uint64_t handler = luaL_checkinteger(vm, 1);

	Sys_LockMutex(_mtx);

	size_t id = Rt_ArrayFindId(&_handlerData, &handler, Rt_U64CmpFunc);
	Sys_ZeroMemory(Rt_ArrayGet(&_handlerData, id), sizeof(struct HandlerData));
	Rt_QueuePush(&_freeHandlers, &id);

	Sys_UnlockMutex(_mtx);

	E_UnregisterHandler(luaL_checkinteger(vm, 1));
	return 0;
}

NE_ENGINE_IF_MOD(Event)
{
	Sys_InitMutex(&_mtx);

	if (!_handlerData.data) {
		Rt_InitArray(&_handlerData, 10, sizeof(struct HandlerData), MH_Script);
		Rt_InitQueue(&_freeHandlers, 10, sizeof(size_t), MH_Script);
		E_RegisterHandler("shutdown", _CleanupHandlerData, NULL);
	}

	luaL_Reg reg[] =
	{
		SIF_REG(Broadcast),
		SIF_REG(RegisterHandler),
		SIF_REG(UnregisterHandler),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Event");
}

static void
_ScriptEventHandler(struct HandlerData *data, void *args)
{
	lua_rawgeti(data->vm, LUA_REGISTRYINDEX, data->func);

	lua_pushlightuserdata(data->vm, args);
	lua_pushlightuserdata(data->vm, data->user);

	if (lua_pcall(data->vm, 2, 0, 0) && lua_gettop(data->vm)) {
		Sys_LogEntry("EventInterface", LOG_CRITICAL, "Failed to execute Lua event handler: %s", lua_tostring(data->vm, -1));
	//	Logger::Log(EVT_MGR_IFACE_MODULE, LOG_DEBUG, "\n%s", *Script::StackDump(state));
		lua_pop(data->vm, 1);
	}
}

static void
_CleanupHandlerData(void *user, void *args)
{
	Rt_TermArray(&_handlerData);
	Rt_TermQueue(&_freeHandlers);

	Sys_TermMutex(_mtx);
}

/* NekoEngine
 *
 * l_Event.c
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
