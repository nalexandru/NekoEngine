#include <Script/Script.h>
#include <Scene/Light.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

SIF_FUNC(GetColor)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeLight *l = lua_touserdata(vm, 1);

	lua_pushnumber(vm, l->color.x);
	lua_pushnumber(vm, l->color.y);
	lua_pushnumber(vm, l->color.z);

	return 3;
}

SIF_FUNC(SetColor)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeVec3 new =
	{
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	};

	struct NeLight *l = lua_touserdata(vm, 1);

	memcpy(&l->color, &new, sizeof(l->color));
	return 0;
}

SIF_FUNC(GetIntensity)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");
	struct NeLight *l = lua_touserdata(vm, 1);
	lua_pushnumber(vm, l->intensity);
	return 1;
}

SIF_FUNC(SetIntensity)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");
	struct NeLight *l = lua_touserdata(vm, 1);
	l->intensity = (float)luaL_checknumber(vm, 2);
	return 0;
}

SIF_FUNC(GetType)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");
	struct NeLight *l = lua_touserdata(vm, 1);
	lua_pushinteger(vm, l->type);
	return 1;
}

SIF_FUNC(SetType)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");
	struct NeLight *l = lua_touserdata(vm, 1);
	l->type = (enum NeLightType)luaL_checkinteger(vm, 2);
	return 0;
}

void
SIface_OpenLight(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(GetColor),
		SIF_REG(SetColor),
		SIF_REG(GetIntensity),
		SIF_REG(SetIntensity),
		SIF_REG(GetType),
		SIF_REG(SetType),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Light");

	lua_pushinteger(vm, LT_DIRECTIONAL);
	lua_setglobal(vm, "LT_DIRECTIONAL");

	lua_pushinteger(vm, LT_POINT);
	lua_setglobal(vm, "LT_POINT");

	lua_pushinteger(vm, LT_SPOT);
	lua_setglobal(vm, "LT_SPOT");
}

/* NekoEngine
 *
 * l_Light.c
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
