#include <Scene/Transform.h>
#include <Scene/Components.h>
#include <Script/Interface.h>

#include "EngineInterface.h"

/*SIF_FUNC(GetRotationAngles)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	float x, y, z;
	const struct NeTransform *xform = (struct NeTransform *)lua_touserdata(vm, 1);
	Xform_RotationAnglesF(xform, &x, &y, &z);

	lua_pushnumber(vm, x);
	lua_pushnumber(vm, y);
	lua_pushnumber(vm, z);

	return 3;
}*/

SIF_FUNC(Move)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);

	struct NeVec3 mvmt;
	if (XMFLOAT3 *v = (XMFLOAT3 *)luaL_testudata(vm, 2, SIF_NE_VEC3); v) {
		memcpy(&mvmt, v, sizeof(*v));
	} else {
		mvmt.x = luaL_checknumber(vm, 3);
		mvmt.y = luaL_checknumber(vm, 4);
		mvmt.z = luaL_checknumber(vm, 5);
	}

	Xform_Move(xform, &mvmt);
	return 0;
}

SIF_FUNC(Rotate)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	const float angle = luaL_checknumber(vm, 2);

	struct NeVec3 axis;
	if (XMFLOAT3 *v = (XMFLOAT3 *)luaL_testudata(vm, 3, SIF_NE_VEC3); v) {
		memcpy(&axis, v, sizeof(*v));
	} else {
		axis.x = luaL_checknumber(vm, 3);
		axis.y = luaL_checknumber(vm, 4);
		axis.z = luaL_checknumber(vm, 5);
	}

	Xform_Rotate(xform, angle, &axis);
	return 0;
}

SIF_FUNC(AdjustScale)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);

	struct NeVec3 scale;
	if (XMFLOAT3 *v = (XMFLOAT3 *)luaL_testudata(vm, 2, SIF_NE_VEC3); v) {
		memcpy(&scale, v, sizeof(*v));
	} else {
		scale.x = luaL_checknumber(vm, 3);
		scale.y = luaL_checknumber(vm, 4);
		scale.z = luaL_checknumber(vm, 5);
	}

	Xform_Scale(xform, &scale);
	return 0;
}

SIF_FUNC(Forward)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	memcpy(v, &xform->forward, sizeof(*v));
	return 1;
}

SIF_FUNC(Right)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	memcpy(v, &xform->right, sizeof(*v));
	return 1;
}

SIF_FUNC(Up)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	memcpy(v, &xform->up, sizeof(*v));
	return 1;
}

SIF_FUNC(Dirty)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	lua_pushboolean(vm, xform->dirty);
	return 1;
}

SIF_FUNC(Position)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);

	struct NeVec3 pos;
	if (XMFLOAT3 *v = (XMFLOAT3 *)luaL_testudata(vm, 2, SIF_NE_VEC3); v) {
		memcpy(&pos, v, sizeof(*v));
		Xform_SetPosition(xform, &pos);
	} else if (lua_isnumber(vm, 2)) {
		pos.x = luaL_checknumber(vm, 2);
		pos.y = luaL_checknumber(vm, 3);
		pos.z = luaL_checknumber(vm, 4);
		Xform_SetPosition(xform, &pos);
	} else {
		v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
		luaL_setmetatable(vm, SIF_NE_VEC3);
		memcpy(v, &xform->position, sizeof(*v));
		return 1;
	}

	return 0;
}

SIF_FUNC(WorldPosition)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);

	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);

	struct NeVec3 vec;
	Xform_Position(xform, &vec);

	memcpy(v, &vec, sizeof(*v));
	return 1;
}

SIF_FUNC(Rotation)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);

	struct NeQuaternion rot;
	if (XMFLOAT4 *v = (XMFLOAT4 *)luaL_testudata(vm, 2, SIF_NE_QUATERNION); v) {
		memcpy(&rot, v, sizeof(*v));
		Xform_SetRotation(xform, &rot);
	} else if (lua_isnumber(vm, 2)) {
		rot.x = luaL_checknumber(vm, 2);
		rot.y = luaL_checknumber(vm, 3);
		rot.z = luaL_checknumber(vm, 4);
		rot.w = luaL_checknumber(vm, 5);
		Xform_SetRotation(xform, &rot);
	} else {
		v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
		luaL_setmetatable(vm, SIF_NE_QUATERNION);
		memcpy(v, &xform->rotation, sizeof(*v));
		return 1;
	}

	return 0;
}

SIF_FUNC(WorldRotation)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);

	struct NeQuaternion quat;
	Xform_Rotation(xform, &quat);

	memcpy(v, &quat, sizeof(*v));
	return 1;
}

SIF_FUNC(Scale)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);

	struct NeVec3 scale;
	if (XMFLOAT3 *v = (XMFLOAT3 *)luaL_testudata(vm, 2, SIF_NE_VEC3); v) {
		memcpy(&scale, v, sizeof(*v));
		Xform_SetPosition(xform, &scale);
	} else if (lua_isnumber(vm, 2)) {
		scale.x = luaL_checknumber(vm, 2);
		scale.y = luaL_checknumber(vm, 3);
		scale.z = luaL_checknumber(vm, 4);
		Xform_SetPosition(xform, &scale);
	} else {
		v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
		luaL_setmetatable(vm, SIF_NE_VEC3);
		memcpy(v, &xform->scale, sizeof(*v));
		return 1;
	}

	return 0;
}

SIF_FUNC(WorldScale)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);

	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);

	struct NeVec3 scale;
	Xform_Scale(xform, &scale);

	memcpy(v, &scale, sizeof(*v));
	return 1;
}

SIF_FUNC(MoveForward)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	Xform_MoveForward(xform, luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(MoveBackward)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	Xform_MoveBackward(xform, luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(MoveRight)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	Xform_MoveRight(xform, luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(MoveLeft)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	Xform_MoveLeft(xform, luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(MoveUp)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	Xform_MoveUp(xform, luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(MoveDown)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	Xform_MoveDown(xform, luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(__tostring)
{
	SIF_CHECKCOMPONENT(1, xform, NE_TRANSFORM, struct NeTransform *);
	lua_pushfstring(vm, "NeTransform(p(%f, %f, %f), r(%f, %f, %f, %f) s(%f %f %f))",
					xform->position.x, xform->position.y, xform->position.z,
					xform->rotation.x, xform->rotation.y, xform->rotation.z, xform->rotation.w,
					xform->scale.x, xform->scale.y, xform->scale.z);
	return 1;
}

NE_ENGINE_IF_MOD(Transform)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(Move),
		SIF_REG(Rotate),
		SIF_REG(AdjustScale),
		SIF_REG(Forward),
		SIF_REG(Right),
		SIF_REG(Up),
		SIF_REG(MoveForward),
		SIF_REG(MoveBackward),
		SIF_REG(MoveRight),
		SIF_REG(MoveLeft),
		SIF_REG(MoveUp),
		SIF_REG(MoveDown),
		SIF_REG(Dirty),
		SIF_REG(Position),
		SIF_REG(WorldPosition),
		SIF_REG(Rotation),
		SIF_REG(WorldRotation),
		SIF_REG(Scale),
		SIF_REG(WorldScale),
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, NE_TRANSFORM);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);
}

/* NekoEngine
 *
 * l_Transform.cxx
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
