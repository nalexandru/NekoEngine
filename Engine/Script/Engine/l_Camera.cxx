#include <Math/Math.h>
#include <Scene/Camera.h>
#include <Scene/Components.h>
#include <Script/Interface.h>

#include "EngineInterface.h"

SIF_FUNC(Rotation)
{
	SIF_CHECKCOMPONENT(1, cam, NE_CAMERA, struct NeCamera *);

	struct NeVec3 rot;
	if (XMFLOAT3 *v = (XMFLOAT3 *)luaL_testudata(vm, 2, SIF_NE_VEC3); v) {
		memcpy(&rot, v, sizeof(*v));
		memcpy(&cam->rotation, &rot, sizeof(cam->rotation));
	} else if (lua_isnumber(vm, 2)) {
		rot.x = luaL_checknumber(vm, 2);
		rot.y = luaL_checknumber(vm, 3);
		rot.z = luaL_checknumber(vm, 4);
		memcpy(&cam->rotation, &rot, sizeof(cam->rotation));
	} else {
		v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
		luaL_setmetatable(vm, SIF_NE_VEC3);
		memcpy(v, &cam->rotation, sizeof(*v));
		return 1;
	}

	return 0;
}

SIF_FUNC(FieldOfView)
{
	SIF_CHECKCOMPONENT(1, cam, NE_CAMERA, struct NeCamera *);
	if (lua_isnumber(vm, 2)) {
		cam->fov = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, cam->fov);
		return 1;
	}
}

SIF_FUNC(Near)
{
	SIF_CHECKCOMPONENT(1, cam, NE_CAMERA, struct NeCamera *);
	if (lua_isnumber(vm, 2)) {
		cam->zNear = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, cam->zNear);
		return 1;
	}
}

SIF_FUNC(Far)
{
	SIF_CHECKCOMPONENT(1, cam, NE_CAMERA, struct NeCamera *);
	if (lua_isnumber(vm, 2)) {
		cam->zFar = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, cam->zFar);
		return 1;
	}
}

SIF_FUNC(ProjectionType)
{
	SIF_CHECKCOMPONENT(1, cam, NE_CAMERA, struct NeCamera *);
	if (lua_isinteger(vm, 2)) {
		const lua_Integer pt = lua_tointeger(vm, 2);
		luaL_argcheck(vm, pt > 0 && pt <= PT_Orthographic, 2, "Must be a valid projection type");
		cam->projection = (NeProjectionType)pt;
		return 0;
	} else {
		lua_pushinteger(vm, cam->projection);
		return 1;
	}
}

SIF_FUNC(__tostring)
{
	SIF_CHECKCOMPONENT(1, cam, NE_CAMERA, struct NeCamera *);
	lua_pushfstring(vm, "NeCamera(%s, %f, %f, %f)",
					cam->projection == PT_Perspective ? "Perspective" : "Orthographic",
					cam->fov, cam->zNear, cam->zFar);
	return 1;
}

NE_ENGINE_IF_MOD(Camera)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(Rotation),
		SIF_REG(FieldOfView),
		SIF_REG(Near),
		SIF_REG(Far),
		SIF_REG(ProjectionType),
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, NE_CAMERA);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	lua_newtable(vm);
	{
		lua_pushinteger(vm, PT_Perspective);
		lua_setfield(vm, -2, "Perspective");

		lua_pushinteger(vm, PT_Orthographic);
		lua_setfield(vm, -2, "Orthographic");
	}
	lua_setglobal(vm, "ProjectionType");
}

/* NekoEngine
 *
 * l_Camera.cxx
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
