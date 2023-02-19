#include <Script/Script.h>
#include <Scene/Camera.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

SIF_FUNC(GetRotation)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	const struct NeCamera *cam = lua_touserdata(vm, 1);

	lua_pushnumber(vm, cam->rotation.x);
	lua_pushnumber(vm, cam->rotation.y);
	lua_pushnumber(vm, cam->rotation.z);

	return 3;
}

SIF_FUNC(SetRotation)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	const struct NeVec3 new =
	{
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	};

	struct NeCamera *cam = lua_touserdata(vm, 1);

	memcpy(&cam->rotation, &new, sizeof(cam->rotation));
	return 0;
}

SIF_FUNC(GetFOV)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	const struct NeCamera *cam = lua_touserdata(vm, 1);
	lua_pushnumber(vm, cam->fov);

	return 1;
}

SIF_FUNC(SetFOV)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeCamera *cam = lua_touserdata(vm, 1);
	cam->fov = (float)luaL_checknumber(vm, 2);

	return 0;
}

SIF_FUNC(GetNear)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	const struct NeCamera *cam = lua_touserdata(vm, 1);
	lua_pushnumber(vm, cam->zNear);

	return 1;
}

SIF_FUNC(SetNear)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeCamera *cam = lua_touserdata(vm, 1);
	cam->zNear = (float)luaL_checknumber(vm, 2);

	return 0;
}

SIF_FUNC(GetFar)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	const struct NeCamera *cam = lua_touserdata(vm, 1);
	lua_pushnumber(vm, cam->zFar);

	return 1;
}

SIF_FUNC(SetFar)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeCamera *cam = lua_touserdata(vm, 1);
	cam->zFar = (float)luaL_checknumber(vm, 2);

	return 0;
}

SIF_FUNC(GetProjectionType)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	const struct NeCamera *cam = lua_touserdata(vm, 1);
	lua_pushinteger(vm, cam->projection);

	return 1;
}

SIF_FUNC(SetProjectionType)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeCamera *cam = lua_touserdata(vm, 1);
	cam->projection = (enum NeProjectionType)luaL_checkinteger(vm, 2);

	return 0;
}

void
SIface_OpenCamera(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(GetRotation),
		SIF_REG(SetRotation),
		SIF_REG(GetFOV),
		SIF_REG(SetFOV),
		SIF_REG(GetNear),
		SIF_REG(SetNear),
		SIF_REG(GetFar),
		SIF_REG(SetFar),
		SIF_REG(GetProjectionType),
		SIF_REG(SetProjectionType),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Camera");

	lua_pushinteger(vm, PT_Perspective);
	lua_setglobal(vm, "PT_Perspective");

	lua_pushinteger(vm, PT_Orthographic);
	lua_setglobal(vm, "PT_Orthographic");
}

/* NekoEngine
 *
 * l_Camera.c
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
