#include <Script/Script.h>
#include <Engine/Entity.h>
#include <Engine/Engine.h>
#include <Engine/Version.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

SIF_LUSRDATA(Screen, E_screen);
SIF_NUMBER(ScreenWidth, *E_screenWidth);
SIF_NUMBER(ScreenHeight, *E_screenHeight);
SIF_NUMBER(DeltaTime, E_deltaTime);
SIF_NUMBER(Time, E_Time());
SIF_STRING(Version, E_VER_STR);
SIF_INTEGER(ScriptAPI, SCRIPT_API_VERSION);

// Entity

SIF_FUNC(FindEntity)
{
	void *p = E_FindEntity(luaL_checkstring(vm, 1));
	if (p)
		lua_pushlightuserdata(vm, p);
	else
		lua_pushnil(vm);

	return 1;
}

SIF_FUNC(CreateEntity)
{
	const char *name = NULL, *type = NULL;

	if (lua_isstring(vm, 1))
		name = lua_tostring(vm, 1);
	else if (!lua_isnil(vm, 1))
		luaL_argerror(vm, 1, "name must be a string");

	if (lua_isstring(vm, 2))
		type = lua_tostring(vm, 2);
	else if (!lua_isnil(vm, 2))
		luaL_argerror(vm, 2, "type must be a string");

	lua_pushlightuserdata(vm, E_CreateEntity(name, type));
	return 1;
}

SIF_FUNC(CreateEntityS)
{
	const char *name = NULL, *type = NULL;

	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (lua_isstring(vm, 2))
		name = lua_tostring(vm, 2);
	else if (!lua_isnil(vm, 2))
		luaL_argerror(vm, 2, "name must be a string");

	if (lua_isstring(vm, 3))
		type = lua_tostring(vm, 3);
	else if (!lua_isnil(vm, 3))
		luaL_argerror(vm, 3, "type must be a string");

	lua_pushlightuserdata(vm, E_CreateEntityS(lua_touserdata(vm, 1), name, type));
	return 1;
}

SIF_FUNC(AddComponent)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 3))
		luaL_argerror(vm, 3, "");

	lua_pushboolean(vm, E_AddComponent(lua_touserdata(vm, 1), E_ComponentTypeId(luaL_checkstring(vm, 2)), (NeCompHandle)lua_touserdata(vm, 3)));
	return 1;
}

SIF_FUNC(AddNewComponent)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushboolean(vm, E_AddNewComponent(lua_touserdata(vm, 1), E_ComponentTypeId(luaL_checkstring(vm, 2)), NULL));
	return 1;
}

SIF_FUNC(RemoveComponent)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	E_RemoveComponent(lua_touserdata(vm, 1), luaL_checkinteger(vm, 2));
	return 0;
}

SIF_FUNC(DestroyEntity)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	E_DestroyEntity(lua_touserdata(vm, 1));
	return 0;
}

SIF_FUNC(EntityName)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushstring(vm, E_EntityName(lua_touserdata(vm, 1)));
	return 1;
}

// Component

SIF_FUNC(GetComponent)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	NeCompTypeId type = E_ComponentTypeId(luaL_checkstring(vm, 2));
	if (type == RT_NOT_FOUND)
		luaL_argerror(vm, 2, "The specified component does not exist");

	void *p = E_GetComponent((NeEntityHandle)lua_touserdata(vm, 1), type);
	if (p)
		lua_pushlightuserdata(vm, p);
	else
		lua_pushnil(vm);

	return 1;
}

SIF_FUNC(ComponentOwner)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	void *p = E_ComponentOwner((NeCompHandle)lua_touserdata(vm, 1));
	if (p)
		lua_pushlightuserdata(vm, p);
	else
		lua_pushnil(vm);

	return 1;
}

SIF_FUNC(ComponentType)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushinteger(vm, E_ComponentType((NeCompHandle)lua_touserdata(vm, 1)));
	return 1;
}

SIF_FUNC(ComponentTypeId)
{
	lua_pushinteger(vm, E_ComponentTypeId(luaL_checkstring(vm, 1)));
	return 1;
}

SIF_FUNC(ComponentCount)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushinteger(vm, E_ComponentCount((NeCompHandle)lua_touserdata(vm, 1)));
	return 1;
}

// TODO: E_GetAllComponents

SIF_FUNC(Shutdown)
{
	E_Shutdown();
	return 0;
}

void
SIface_OpenEngine(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		// Entity
		SIF_REG(FindEntity),
		SIF_REG(CreateEntity),
		SIF_REG(CreateEntityS),
		SIF_REG(AddComponent),
		SIF_REG(AddNewComponent),
		SIF_REG(RemoveComponent),
		SIF_REG(DestroyEntity),
		SIF_REG(EntityName),

		// Component
		SIF_REG(GetComponent),
		SIF_REG(ComponentOwner),
		SIF_REG(ComponentType),
		SIF_REG(ComponentTypeId),
		SIF_REG(ComponentCount),

		SIF_REG(Screen),
		SIF_REG(ScreenWidth),
		SIF_REG(ScreenHeight),
		SIF_REG(DeltaTime),
		SIF_REG(Time),
		SIF_REG(Version),
		SIF_REG(ScriptAPI),
		SIF_REG(Shutdown),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "E");
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
