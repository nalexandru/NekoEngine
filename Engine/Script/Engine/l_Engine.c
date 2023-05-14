#include <Script/Script.h>
#include <Script/Interface.h>
#include <Engine/Job.h>
#include <Engine/Plugin.h>
#include <Engine/Entity.h>
#include <Engine/Engine.h>
#include <Engine/Version.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>
#include <Engine/ECSystem.h>
#include <System/Log.h>

#include "EngineInterface.h"

struct NeArray ScEngineInterface_modules = { 0 };

SIF_LUSRDATA(Screen, E_screen);
SIF_INTEGER(ScreenWidth, *E_screenWidth);
SIF_INTEGER(ScreenHeight, *E_screenHeight);
SIF_NUMBER(DeltaTime, (float)E_deltaTime);
SIF_INTEGER(WorkerId, E_WorkerId());
SIF_NUMBER(Time, (float)E_Time());
SIF_INTEGER(JobWorkerThreads, E_JobWorkerThreads());
SIF_BOOL(PluginLoaded, E_PluginLoaded(luaL_checkstring(vm, 1)));

// Entity

SIF_FUNC(FindEntity)
{
	void *p = E_FindEntity(luaL_checkstring(vm, 1));
	if (p)
		Sc_PushScriptWrapper(vm, p, SIF_NE_ENTITY);
	else
		lua_pushnil(vm);
	return 1;
}

SIF_FUNC(SendMessage)
{
	SIF_CHECKCOMPONENT(1, ent, SIF_NE_ENTITY, NeEntityHandle);

	const uint32_t msg = (uint32_t)luaL_checkinteger(vm, 2);
	const void *data = lua_islightuserdata(vm, 3) ? lua_touserdata(vm, 3) : NULL;

	E_SendMessage(ent, msg, data);
	return 0;
}

SIF_FUNC(CreateEntity)
{
	const char *name = luaL_checkstring(vm, 1), *type = luaL_checkstring(vm, 2);

	NeEntityHandle ent = E_CreateEntity(name, type);
	if (!ent)
		luaL_error(vm, "Failed to create entity");

	Sc_PushScriptWrapper(vm, ent, SIF_NE_ENTITY);
	return 1;
}

SIF_FUNC(AddComponent)
{
	SIF_CHECKCOMPONENT(1, ent, SIF_NE_ENTITY, NeEntityHandle);

	NeCompTypeId type = E_ComponentTypeId(luaL_checkstring(vm, 2));
	if (type == (NeCompTypeId)-1)
		luaL_argerror(vm, 2, "Component type not found");

	void **args = NULL;
	const uint32_t argc = lua_istable(vm, 3) ? (uint32_t)lua_rawlen(vm, 3) : 0;
	if (argc) {
		args = Sys_Alloc(sizeof(*args), argc + 1, MH_Script);
		for (uint32_t i = 0; i < argc; ++i) {
			lua_rawgeti(vm, 3, i + 1);
			int v = lua_gettop(vm);
			args[i] = Rt_StrDup(luaL_checkstring(vm, v), MH_Script);
			lua_remove(vm, v);
		}
	}

	lua_pushboolean(vm, E_AddNewComponent(ent, type, (const void **)args));

	for (uint32_t i = 0; i < argc; ++i)
		Sys_Free(args[i]);
	Sys_Free(args);

	return 1;
}

SIF_FUNC(RemoveComponent)
{
	SIF_CHECKCOMPONENT(1, ent, SIF_NE_ENTITY, NeEntityHandle);

	NeCompTypeId type = lua_isinteger(vm, 2) ? lua_tointeger(vm, 2) : E_ComponentTypeId(luaL_checkstring(vm, 2));
	if (type == RT_NOT_FOUND)
		luaL_argerror(vm, 2, "The specified component does not exist");

	E_RemoveComponent(ent, type);
	return 0;
}

SIF_FUNC(Destroy)
{
	SIF_CHECKCOMPONENT(1, ent, SIF_NE_ENTITY, NeEntityHandle);
	E_DestroyEntity(ent);
	return 0;
}

SIF_FUNC(Name)
{
	SIF_CHECKCOMPONENT(1, ent, SIF_NE_ENTITY, NeEntityHandle);
	lua_pushstring(vm, E_EntityName(ent));
	return 1;
}

SIF_FUNC(Component)
{
	SIF_CHECKCOMPONENT(1, ent, SIF_NE_ENTITY, NeEntityHandle);

	NeCompTypeId type;
	const char *typeName;

	if (lua_isinteger(vm, 2)) {
		type = lua_tointeger(vm, 2);
		typeName = E_ComponentTypeName(type);
	} else {
		typeName = luaL_checkstring(vm, 2);
		type = E_ComponentTypeId(typeName);
	}

	if (type == RT_NOT_FOUND || typeName == NULL)
		luaL_argerror(vm, 2, "The specified component does not exist");

	Sc_PushScriptWrapper(vm, E_GetComponent(ent, type), typeName);
	return 1;
}

SIF_FUNC(ComponentCount)
{
	NeCompTypeId type = lua_isinteger(vm, 2) ? lua_tointeger(vm, 2) : E_ComponentTypeId(luaL_checkstring(vm, 2));
	if (type == RT_NOT_FOUND)
		luaL_argerror(vm, 2, "The specified component does not exist");

	lua_pushinteger(vm, E_ComponentCount(type));
	return 1;
}

// TODO: E_GetAllComponents

SIF_FUNC(Shutdown)
{
	E_Shutdown();
	return 0;
}

SIF_FUNC(Log)
{
	const char *module = luaL_checkstring(vm, 1);
	const int severity = (int)luaL_checkinteger(vm, 2);
	const char *message = luaL_checkstring(vm, 3);

	if (!module || !message)
		return 0;

	Sys_LogEntry(module, severity, message);

	return 0;
}

SIF_FUNC(__tostring)
{
	SIF_TESTCOMPONENT(1, ent, SIF_NE_ENTITY, NeEntityHandle);
	if (ent) {
		lua_pushfstring(vm, "NeEntity(%s)", E_EntityName(ent));
		return 1;
	}

	return 0;
}

NE_SCRIPT_INTEFACE(NeEngine)
{
	void (*func)(lua_State *vm);
	Rt_ArrayForEachPtr(func, &ScEngineInterface_modules)
		func(vm);

	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(Component),
		SIF_REG(AddComponent),
		SIF_REG(RemoveComponent),
		SIF_REG(SendMessage),
		SIF_REG(Destroy),
		SIF_REG(Name),
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, SIF_NE_ENTITY);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	luaL_Reg reg[] =
	{
		// Entity
		SIF_REG(FindEntity),
		SIF_REG(SendMessage),
		SIF_REG(CreateEntity),

		// Component
		SIF_REG(ComponentCount),

		SIF_REG(Screen),
		SIF_REG(ScreenWidth),
		SIF_REG(ScreenHeight),
		SIF_REG(DeltaTime),
		SIF_REG(Time),
		SIF_REG(Log),
		SIF_REG(WorkerId),
		SIF_REG(JobWorkerThreads),
		SIF_REG(PluginLoaded),
		SIF_REG(Shutdown),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Engine");

	lua_pushliteral(vm, E_VER_STR);
	lua_setglobal(vm, "NE_VERSION");

	lua_pushinteger(vm, SCRIPT_API_VERSION);
	lua_setglobal(vm, "NE_SCRIPT_API");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, ECSYS_GROUP_MANUAL_HASH);
		lua_setfield(vm, -2, "Manual");

		lua_pushinteger(vm, ECSYS_GROUP_LOGIC_HASH);
		lua_setfield(vm, -2, "Logic");

		lua_pushinteger(vm, ECSYS_GROUP_POST_LOGIC_HASH);
		lua_setfield(vm, -2, "PostLogic");

		lua_pushinteger(vm, ECSYS_GROUP_PRE_RENDER_HASH);
		lua_setfield(vm, -2, "PreRender");

		lua_pushinteger(vm, ECSYS_GROUP_POST_RENDER_HASH);
		lua_setfield(vm, -2, "PostRender");
	}
	lua_setglobal(vm, "SystemGroup");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, LOG_DEBUG);
		lua_setfield(vm, -2, "Debug");

		lua_pushinteger(vm, LOG_CRITICAL);
		lua_setfield(vm, -2, "Critical");

		lua_pushinteger(vm, LOG_WARNING);
		lua_setfield(vm, -2, "Warning");

		lua_pushinteger(vm, LOG_INFORMATION);
		lua_setfield(vm, -2, "Information");
	}
	lua_setglobal(vm, "Log");

	return 1;
}

/* NekoEngine
 *
 * l_Engine.c
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
