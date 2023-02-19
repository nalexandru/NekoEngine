#include <UI/UI.h>
#include <Script/Script.h>
#include <Engine/Console.h>
#include <Math/Math.h>

#include "Interface.h"

SIF_FUNC(Create)
{
	float m[16];
	for (int i = 0; i < 16; ++i)
		m[i] = (float)luaL_checknumber(vm, i + 1);

	struct NeMatrix *v = (struct NeMatrix *)Sys_Alloc(sizeof(*v), 1, MH_Script);
	if (v) {
		memcpy(v, m, sizeof(*v));
		lua_pushlightuserdata(vm, v);
	} else {
		lua_pushnil(vm);
	}

	return 1;
}

SIF_FUNC(CreateTemp)
{
	float m[16];
	for (int i = 0; i < 16; ++i)
		m[i] = (float)luaL_checknumber(vm, i + 1);

	struct NeMatrix *v = (struct NeMatrix *)Sys_Alloc(sizeof(*v), 1, MH_Transient);
	if (v) {
		memcpy(v, m, sizeof(*v));
		lua_pushlightuserdata(vm, v);
	} else {
		lua_pushnil(vm);
	}

	return 1;
}

SIF_FUNC(Set)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	float m[16];
	for (int i = 0; i < 16; ++i)
		m[i] = (float)luaL_checknumber(vm, i + 1);

	struct NeMatrix *v = (struct NeMatrix *)lua_touserdata(vm, 1);
	memcpy(v, m, sizeof(*v));

	lua_pushlightuserdata(vm, v);
	return 1;
}

SIF_FUNC(Get)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	struct NeMatrix *m = (struct NeMatrix *)lua_touserdata(vm, 1);

	for (int i = 0; i < 16; ++i)
		lua_pushnumber(vm, m->m[i]);

	return 16;
}

SIF_FUNC(GetRows)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	struct NeMatrix *m = (struct NeMatrix *)lua_touserdata(vm, 1);

	lua_pushlightuserdata(vm, &m->r[0]);
	lua_pushlightuserdata(vm, &m->r[1]);
	lua_pushlightuserdata(vm, &m->r[2]);
	lua_pushlightuserdata(vm, &m->r[2]);

	return 4;
}

SIF_FUNC(Copy)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	struct NeMatrix *m1 = (struct NeMatrix *)lua_touserdata(vm, 1);
	struct NeMatrix *m2 = (struct NeMatrix *)lua_touserdata(vm, 2);
	memcpy(m1, m2, sizeof(*m1));

	lua_pushlightuserdata(vm, m1);
	return 1;
}

SIF_FUNC(Mul)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	XMFLOAT4X4A *m1 = (XMFLOAT4X4A *)lua_touserdata(vm, 1);
	XMFLOAT4X4A *m2 = (XMFLOAT4X4A *)lua_touserdata(vm, 2);

	XMStoreFloat4x4A(m1, XMMatrixMultiply(XMLoadFloat4x4A(m1), XMLoadFloat4x4A(m2)));

	lua_pushlightuserdata(vm, m1);
	return 1;
}

SIF_FUNC(MulS)
{
	XMFLOAT4X4A *m = (XMFLOAT4X4A *)lua_touserdata(vm, 1);
	const float f = (float)luaL_checknumber(vm, 2);

	XMMATRIX m1 = XMLoadFloat4x4A(m);
	XMMATRIX m2 = XMMatrixSet(f, f, f, f, f, f, f, f, f, f, f, f, f, f, f, f);

	XMStoreFloat4x4A(m, XMMatrixMultiply(m1, m2));

	lua_pushlightuserdata(vm, m);
	return 1;
}

SIF_FUNC(Inverse)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMFLOAT4X4A *m = (XMFLOAT4X4A *)lua_touserdata(vm, 1);
	XMStoreFloat4x4A(m, XMMatrixInverse(NULL, XMLoadFloat4x4A(m)));

	lua_pushlightuserdata(vm, m);
	return 1;
}

SIF_FUNC(Transpose)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMFLOAT4X4A *m = (XMFLOAT4X4A *)lua_touserdata(vm, 1);
	XMStoreFloat4x4A(m, XMMatrixTranspose(XMLoadFloat4x4A(m)));

	lua_pushlightuserdata(vm, m);
	return 1;
}

SIF_FUNC(Identity)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMFLOAT4X4A *m = (XMFLOAT4X4A *)lua_touserdata(vm, 1);
	XMStoreFloat4x4A(m, XMMatrixIdentity());

	lua_pushlightuserdata(vm, m);
	return 1;
}

SIF_FUNC(LookAt)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	const XMVECTOR eye = XMLoadFloat3A((XMFLOAT3A *)lua_touserdata(vm, 2));
	const XMVECTOR front = XMLoadFloat3A((XMFLOAT3A *)lua_touserdata(vm, 3));
	const XMVECTOR up = XMLoadFloat3A((XMFLOAT3A *)lua_touserdata(vm, 4));

	XMFLOAT4X4A *m = (XMFLOAT4X4A *)lua_touserdata(vm, 1);
	XMStoreFloat4x4A(m, XMMatrixLookAtRH(eye, front, up));

	lua_pushlightuserdata(vm, m);

	return 1;
}

SIF_FUNC(Scale)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMFLOAT4X4A *m = (XMFLOAT4X4A *)lua_touserdata(vm, 1);
	XMStoreFloat4x4A(m, XMMatrixScaling(
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	));

	lua_pushlightuserdata(vm, m);

	return 1;
}

SIF_FUNC(Translation)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMStoreFloat4x4A((XMFLOAT4X4A *)lua_touserdata(vm, 1), XMMatrixTranslation(
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	));

	return 1;
}

SIF_FUNC(Rotation)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMStoreFloat4x4A((XMFLOAT4X4A *)lua_touserdata(vm, 1), XMMatrixRotationRollPitchYaw(
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	));

	return 1;
}

SIF_FUNC(RotationFromQuat)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	XMFLOAT4X4A *m = (XMFLOAT4X4A *)lua_touserdata(vm, 1);
	XMStoreFloat4x4A(m, XMMatrixRotationQuaternion(XMLoadFloat4A((XMFLOAT4A *)lua_touserdata(vm, 2))));

	lua_pushlightuserdata(vm, m);

	return 1;
}

SIF_FUNC(Perspective)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMFLOAT4X4A *m = (XMFLOAT4X4A *)lua_touserdata(vm, 1);
	XMStoreFloat4x4A(m, XMMatrixPerspectiveFovRH((float)luaL_checknumber(vm, 2), (float)luaL_checknumber(vm, 3), (float)luaL_checknumber(vm, 4), (float)luaL_checknumber(vm, 5)));

	lua_pushlightuserdata(vm, m);

	return 1;
}

SIF_FUNC(Orthographic)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	XMFLOAT4X4A *m = (XMFLOAT4X4A *)lua_touserdata(vm, 1);
	XMStoreFloat4x4A(m, XMMatrixOrthographicRH((float)luaL_checknumber(vm, 2), (float)luaL_checknumber(vm, 3), (float)luaL_checknumber(vm, 4), (float)luaL_checknumber(vm, 5)));

	lua_pushlightuserdata(vm, m);

	return 1;
}

/*SIF_FUNC(MulVec4)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	lua_pushnumber(vm, M_MulMatrixVec4(lua_touserdata(vm, 1), lua_touserdata(vm, 2)));
	return 1;
}*/

SIF_FUNC(Destroy)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	Sys_Free(lua_touserdata(vm, 1));
	return 0;
}

void
SIface_OpenMatrix(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(Mul),
		SIF_REG(MulS),
		SIF_REG(Inverse),
		SIF_REG(Transpose),
		SIF_REG(Identity),
		SIF_REG(LookAt),
		SIF_REG(Scale),
		SIF_REG(Translation),
		SIF_REG(Rotation),
		SIF_REG(RotationFromQuat),
		SIF_REG(Perspective),
		SIF_REG(Orthographic),
		SIF_REG(Copy),
		SIF_REG(Create),
		SIF_REG(CreateTemp),
		SIF_REG(Set),
		SIF_REG(Get),
		SIF_REG(GetRows),
		SIF_REG(Destroy),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Matrix");
}

/* NekoEngine
 *
 * l_Matrix.c
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
