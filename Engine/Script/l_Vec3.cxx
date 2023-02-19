#include <UI/UI.h>
#include <Script/Script.h>
#include <Engine/Console.h>
#include <Math/Math.h>

#include "Interface.h"

SIF_FUNC(Create)
{
	const float x = (float)luaL_checknumber(vm, 1);
	const float y = (float)luaL_checknumber(vm, 2);
	const float z = (float)luaL_checknumber(vm, 3);

	struct NeVec3 *v = (struct NeVec3 *)Sys_Alloc(sizeof(*v), 1, MH_Script);
	if (v) {
		v->x = x; v->y = y; v->z = z;
		lua_pushlightuserdata(vm, v);
	} else {
		lua_pushnil(vm);
	}

	return 1;
}

SIF_FUNC(CreateTemp)
{
	const float x = (float)luaL_checknumber(vm, 1);
	const float y = (float)luaL_checknumber(vm, 2);
	const float z = (float)luaL_checknumber(vm, 3);

	struct NeVec3 *v = (struct NeVec3 *)Sys_Alloc(sizeof(*v), 1, MH_Transient);
	if (v) {
		v->x = x; v->y = y; v->z = z;
		lua_pushlightuserdata(vm, v);
	} else {
		lua_pushnil(vm);
	}

	return 1;
}
#include <System/Log.h>
SIF_FUNC(Set)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	struct NeVec3 *v = (struct NeVec3 *)lua_touserdata(vm, 1);
	v->x = (float)luaL_checknumber(vm, 2);
	v->y = (float)luaL_checknumber(vm, 3);
	v->z = (float)luaL_checknumber(vm, 4);

	Sys_LogEntry("AA", LOG_DEBUG, "%.02f, %.02f, %.02f", v->x, v->y, v->z);

	lua_pushlightuserdata(vm, v);

	return 1;
}

SIF_FUNC(Get)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	const struct NeVec3 *v = (const struct NeVec3 *)lua_touserdata(vm, 1);

	lua_pushnumber(vm, v->x);
	lua_pushnumber(vm, v->y);
	lua_pushnumber(vm, v->z);

	return 3;
}

SIF_FUNC(Fill)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	struct NeVec3 *v = (struct NeVec3 *)lua_touserdata(vm, 1);
	v->x = v->y = v->z = (float)luaL_checknumber(vm, 2);

	lua_pushlightuserdata(vm, v);
	return 1;
}

SIF_FUNC(Copy)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	memcpy(lua_touserdata(vm, 1), lua_touserdata(vm, 2), sizeof(struct NeVec3));

	lua_pushlightuserdata(vm, lua_touserdata(vm, 1));
	return 1;
}

SIF_FUNC(Zero)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	struct NeVec3 *v = (struct NeVec3 *)lua_touserdata(vm, 1);
	v->x = v->y = v->z = 0.f;

	lua_pushlightuserdata(vm, v);
	return 1;
}

SIF_FUNC(Add)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	XMFLOAT3A *v1 = (XMFLOAT3A *)lua_touserdata(vm, 1);
	XMFLOAT3A *v2 = (XMFLOAT3A *)lua_touserdata(vm, 2);

	XMStoreFloat3A(v1, XMVectorAdd(XMLoadFloat3A(v1), XMLoadFloat3A(v2)));

	lua_pushlightuserdata(vm, v1);
	return 1;
}

SIF_FUNC(AddS)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMFLOAT3A *v1 = (XMFLOAT3A *)lua_touserdata(vm, 1);

	XMStoreFloat3A(v1, XMVectorAdd(XMLoadFloat3A(v1), XMVectorReplicate((float)luaL_checknumber(vm, 2))));

	lua_pushlightuserdata(vm, v1);
	return 1;
}

SIF_FUNC(Sub)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	XMFLOAT3A *v1 = (XMFLOAT3A *)lua_touserdata(vm, 1);
	const XMFLOAT3A *v2 = (const XMFLOAT3A *)lua_touserdata(vm, 2);

	XMStoreFloat3A(v1, XMVectorSubtract(XMLoadFloat3A(v1), XMLoadFloat3A(v2)));

	lua_pushlightuserdata(vm, v1);
	return 1;
}

SIF_FUNC(SubS)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMFLOAT3A *v1 = (XMFLOAT3A *)lua_touserdata(vm, 1);

	XMStoreFloat3A(v1, XMVectorSubtract(XMLoadFloat3A(v1), XMVectorReplicate((float)luaL_checknumber(vm, 2))));

	lua_pushlightuserdata(vm, v1);
	return 1;
}


SIF_FUNC(Mul)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	XMFLOAT3A *v1 = (XMFLOAT3A *)lua_touserdata(vm, 1);
	const XMFLOAT3A *v2 = (const XMFLOAT3A *)lua_touserdata(vm, 2);

	XMStoreFloat3A(v1, XMVectorMultiply(XMLoadFloat3A(v1), XMLoadFloat3A(v2)));

	lua_pushlightuserdata(vm, v1);
	return 1;
}

SIF_FUNC(MulS)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMFLOAT3A *v1 = (XMFLOAT3A *)lua_touserdata(vm, 1);

	XMStoreFloat3A(v1, XMVectorMultiply(XMLoadFloat3A(v1), XMVectorReplicate((float)luaL_checknumber(vm, 2))));

	lua_pushlightuserdata(vm, v1);
	return 1;
}


SIF_FUNC(Div)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	XMFLOAT3A *v1 = (XMFLOAT3A *)lua_touserdata(vm, 1);
	const XMFLOAT3A *v2 = (const XMFLOAT3A *)lua_touserdata(vm, 2);

	XMStoreFloat3A(v1, XMVectorDivide(XMLoadFloat3A(v1), XMLoadFloat3A(v2)));

	lua_pushlightuserdata(vm, v1);
	return 1;
}

SIF_FUNC(DivS)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMFLOAT3A *v1 = (XMFLOAT3A *)lua_touserdata(vm, 1);

	XMStoreFloat3A(v1, XMVectorDivide(XMLoadFloat3A(v1), XMVectorReplicate((float)luaL_checknumber(vm, 2))));

	lua_pushlightuserdata(vm, v1);
	return 1;
}

SIF_FUNC(Dot)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	XMFLOAT3A *v1 = (XMFLOAT3A *)lua_touserdata(vm, 1);
	const XMFLOAT3A *v2 = (const XMFLOAT3A *)lua_touserdata(vm, 2);

	lua_pushnumber(vm, XMVectorGetX(XMVector3Dot(XMLoadFloat3A(v1), XMLoadFloat3A(v2))));
	return 1;
}

SIF_FUNC(Cross)
{
	return 0;
}

SIF_FUNC(Lerp)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	XMFLOAT3A *v1 = (XMFLOAT3A *)lua_touserdata(vm, 1);
	const XMFLOAT3A *v2 = (const XMFLOAT3A *)lua_touserdata(vm, 2);

	XMStoreFloat3A(v1, XMVectorLerp(XMLoadFloat3A(v1), XMLoadFloat3A(v2), (float)luaL_checknumber(vm, 3)));

	lua_pushlightuserdata(vm, v1);
	return 1;
}

SIF_FUNC(Distance)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	const XMVECTOR v1 = XMLoadFloat3A((XMFLOAT3A *)lua_touserdata(vm, 1));
	const XMVECTOR v2 = XMLoadFloat3A((XMFLOAT3A *)lua_touserdata(vm, 2));

	lua_pushnumber(vm, XMVectorGetX(XMVector3Length(XMVectorSubtract(v1, v2))));
	return 1;
}

SIF_FUNC(Length)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushnumber(vm, XMVectorGetX(XMVector3Length(XMLoadFloat3A((XMFLOAT3A *)lua_touserdata(vm, 1)))));
	return 1;
}

SIF_FUNC(LengthSquared)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushnumber(vm, XMVectorGetX(XMVector3LengthSq(XMLoadFloat3A((XMFLOAT3A *)lua_touserdata(vm, 1)))));
	return 1;
}

SIF_FUNC(Normalize)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMFLOAT3A *v1 = (XMFLOAT3A *)lua_touserdata(vm, 1);

	XMStoreFloat3A(v1, XMVector3Normalize(XMLoadFloat3A(v1)));

	lua_pushlightuserdata(vm, v1);
	return 1;
}

SIF_FUNC(Destroy)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	Sys_Free(lua_touserdata(vm, 1));
	return 0;
}

void
SIface_OpenVec3(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(Add),
		SIF_REG(AddS),
		SIF_REG(Sub),
		SIF_REG(SubS),
		SIF_REG(Mul),
		SIF_REG(MulS),
		SIF_REG(Div),
		SIF_REG(DivS),
		SIF_REG(Dot),
		SIF_REG(Cross),
		SIF_REG(Lerp),
		SIF_REG(Length),
		SIF_REG(LengthSquared),
		SIF_REG(Normalize),
		SIF_REG(Distance),
		SIF_REG(Zero),
		SIF_REG(Fill),
		SIF_REG(Copy),
		SIF_REG(Create),
		SIF_REG(CreateTemp),
		SIF_REG(Set),
		SIF_REG(Get),
		SIF_REG(Destroy),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Vec3");
}

/* NekoEngine
 *
 * l_Vec3.c
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
