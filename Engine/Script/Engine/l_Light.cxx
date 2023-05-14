#include <Math/Math.h>
#include <Scene/Light.h>
#include <Scene/Components.h>
#include <Script/Interface.h>
#include <Runtime/Runtime.h>

#include "EngineInterface.h"

SIF_FUNC(Color)
{
	SIF_CHECKCOMPONENT(1, l, NE_LIGHT, struct NeLight *);

	struct NeVec3 color;
	if (XMFLOAT3 *v = (XMFLOAT3 *)luaL_testudata(vm, 2, SIF_NE_VEC3); v) {
		memcpy(&color, v, sizeof(*v));
		memcpy(&l->color, &color, sizeof(l->color));
	} else if (lua_isnumber(vm, 2)) {
		color.x = luaL_checknumber(vm, 2);
		color.y = luaL_checknumber(vm, 3);
		color.z = luaL_checknumber(vm, 4);
		memcpy(&l->color, &color, sizeof(l->color));
	} else {
		v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
		luaL_setmetatable(vm, SIF_NE_VEC3);
		memcpy(v, &l->color, sizeof(*v));
		return 1;
	}

	return 0;
}

SIF_FUNC(Intensity)
{
	SIF_CHECKCOMPONENT(1, l, NE_LIGHT, struct NeLight *);
	if (lua_isnumber(vm, 2)) {
		l->intensity = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, l->intensity);
		return 1;
	}
}

SIF_FUNC(InnerRadius)
{
	SIF_CHECKCOMPONENT(1, l, NE_LIGHT, struct NeLight *);
	if (lua_isnumber(vm, 2)) {
		l->innerRadius = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, l->innerRadius);
		return 1;
	}
}

SIF_FUNC(OuterRadius)
{
	SIF_CHECKCOMPONENT(1, l, NE_LIGHT, struct NeLight *);
	if (lua_isnumber(vm, 2)) {
		l->outerRadius = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, l->outerRadius);
		return 1;
	}
}

SIF_FUNC(InnerCutoff)
{
	SIF_CHECKCOMPONENT(1, l, NE_LIGHT, struct NeLight *);
	if (lua_isnumber(vm, 2)) {
		l->innerCutoff = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, l->innerCutoff);
		return 1;
	}
}

SIF_FUNC(OuterCutoff)
{
	SIF_CHECKCOMPONENT(1, l, NE_LIGHT, struct NeLight *);
	if (lua_isnumber(vm, 2)) {
		l->outerCutoff = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, l->outerCutoff);
		return 1;
	}
}

SIF_FUNC(Type)
{
	SIF_CHECKCOMPONENT(1, l, NE_LIGHT, struct NeLight *);
	if (lua_isinteger(vm, 2)) {
		const lua_Integer lt = lua_tointeger(vm, 2);
		luaL_argcheck(vm, lt > 0 && lt <= LT_Spot, 2, "Must be a valid light type");
		l->type = (NeLightType)lt;
		return 0;
	} else {
		lua_pushinteger(vm, l->type);
		return 1;
	}
}


SIF_FUNC(__tostring)
{
	SIF_CHECKCOMPONENT(1, l, NE_LIGHT, struct NeLight *);

	const char *type = "Directional";
	if (l->type == LT_Point)
		type = "Point";
	else if (l->type == LT_Spot)
		type = "Spot";

	lua_pushfstring(vm, "NeLight(%s, (%f, %f, %f), %f, %f, %f, %f, %f)",
					type, l->color.x, l->color.y, l->color.z, l->intensity,
					l->innerRadius, l->outerRadius, l->innerCutoff, l->outerCutoff);

	return 1;
}

NE_ENGINE_IF_MOD(Light)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(Color),
		SIF_REG(Intensity),
		SIF_REG(InnerRadius),
		SIF_REG(OuterRadius),
		SIF_REG(InnerCutoff),
		SIF_REG(OuterCutoff),
		SIF_REG(Type),
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, NE_LIGHT);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	lua_newtable(vm);
	{
		lua_pushinteger(vm, LT_Directional);
		lua_setfield(vm, -2, "Directional");

		lua_pushinteger(vm, LT_Point);
		lua_setfield(vm, -2, "Point");

		lua_pushinteger(vm, LT_Spot);
		lua_setfield(vm, -2, "Spot");
	}
	lua_setglobal(vm, "LightType");
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
