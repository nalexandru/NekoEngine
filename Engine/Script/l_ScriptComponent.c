#include <string.h>

#include <Scene/Scene.h>
#include <Engine/Types.h>
#include <Engine/Component.h>
#include <Script/Script.h>
#include <System/Memory.h>

#include "Interface.h"
#include "../Engine/ECS.h"

static inline uint8_t *
_DataPtr(lua_State *vm)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	const size_t offset = luaL_checkinteger(vm, 2) + sizeof(struct NeCompBase);
	const size_t fieldSize = luaL_checkinteger(vm, 3);

	struct NeCompBase *comp = lua_touserdata(vm, 1);

	struct NeScene *s = Scn_GetScene((uint8_t)comp->_sceneId);
	if (s->loaded)
		Sys_AtomicLockRead(&s->compLock);

	const size_t compSize = E_ComponentTypeSize(comp->_typeId);
	
	if (s->loaded)
		Sys_AtomicUnlockRead(&s->compLock);

	if (offset >= compSize)
		luaL_argerror(vm, 2, "Offset must be smaller than the component size");

	if (offset + fieldSize > compSize)
		luaL_argerror(vm, 3, "Offset + Size must be smaller than or equal to the component size");

	return (uint8_t *)comp + offset;
}

SIF_FUNC(GetI32)
{
	lua_pushinteger(vm, *(int32_t *)_DataPtr(vm));
	return 1;
}

SIF_FUNC(SetI32)
{
	*(int32_t *)(_DataPtr(vm)) = (int32_t)luaL_checkinteger(vm, 4);
	return 0;
}

SIF_FUNC(GetI64)
{
	lua_pushinteger(vm, *(int64_t *)_DataPtr(vm));
	return 1;
}

SIF_FUNC(SetI64)
{
	*(int64_t *)(_DataPtr(vm)) = (int64_t)luaL_checkinteger(vm, 4);
	return 0;
}

SIF_FUNC(GetFlt)
{
	lua_pushnumber(vm, *(float *)_DataPtr(vm));
	return 1;
}

SIF_FUNC(SetFlt)
{
	*(float *)(_DataPtr(vm)) = (float)luaL_checknumber(vm, 4);
	return 0;
}

SIF_FUNC(GetDbl)
{
	lua_pushnumber(vm, *(double *)_DataPtr(vm));
	return 1;
}

SIF_FUNC(SetDbl)
{
	*(double *)(_DataPtr(vm)) = (double)luaL_checknumber(vm, 4);
	return 0;
}

SIF_FUNC(GetBln)
{
	lua_pushboolean(vm, *(bool *)_DataPtr(vm));
	return 1;
}

SIF_FUNC(SetBln)
{
	if (!lua_isboolean(vm, 4))
		luaL_argerror(vm, 4, "");
	*(bool *)(_DataPtr(vm)) = lua_toboolean(vm, 4);
	return 0;
}

SIF_FUNC(GetStr)
{
	lua_pushstring(vm, *(char **)_DataPtr(vm));
	return 1;
}

SIF_FUNC(SetStr)
{
	char **str = (char **)_DataPtr(vm);
	const char *val = luaL_checkstring(vm, 4);
	size_t len = strlen(val);

	if (*str)
		Sys_Free(*str);
	*str = NULL;

	if (!len)
		return 0;

	*str = Sys_Alloc(sizeof(char), len + 1, MH_Script);
	snprintf(*str, len, "%s", val);

	return 0;
}

SIF_FUNC(GetPtr)
{
	lua_pushlightuserdata(vm, *(void **)_DataPtr(vm));
	return 1;
}

SIF_FUNC(SetPtr)
{
	if (!lua_islightuserdata(vm, 4))
		luaL_argerror(vm, 4, "");
	*(void **)(_DataPtr(vm)) = lua_touserdata(vm, 4);
	return 0;
}

void
SIface_OpenScriptComponent(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(GetI32),
		SIF_REG(SetI32),
		SIF_REG(GetI64),
		SIF_REG(SetI64),
		SIF_REG(GetFlt),
		SIF_REG(SetFlt),
		SIF_REG(GetDbl),
		SIF_REG(SetDbl),
		SIF_REG(GetBln),
		SIF_REG(SetBln),
		SIF_REG(GetStr),
		SIF_REG(SetStr),
		SIF_REG(GetPtr),
		SIF_REG(SetPtr),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "CompIF");
}

/* NekoEngine
 *
 * l_ScriptComponent.c
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
