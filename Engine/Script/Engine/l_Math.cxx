#include <Math/Math.h>
#include <System/Memory.h>
#include <Script/Interface.h>

#include "EngineInterface.h"

void SIface_PushVec2(lua_State *vm, const struct NeVec2 *v2)
{
	XMFLOAT2 *v = (XMFLOAT2 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC2);
	memcpy(v, v2, sizeof(*v));
}

// NeVec3

SIF_FUNC(V3_New)
{
	float x = (float)luaL_optnumber(vm, 1, 0.f);
	float y = (float)luaL_optnumber(vm, 2, 0.f);
	float z = (float)luaL_optnumber(vm, 3, 0.f);

	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);

	v->x = x;
	v->y = y;
	v->z = z;

	return 1;
}

SIF_FUNC(V3_Set)
{
	XMFLOAT3 *v = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);

	if (lua_isnumber(vm, 2)) {
		v->x = v->y = v->z = lua_tonumber(vm, 2);
	} else {
		XMFLOAT3 *v2 = (XMFLOAT3 *)luaL_checkudata(vm, 2, SIF_NE_VEC3);
		memcpy(v, v2, sizeof(*v));
	}

	return 0;
}

SIF_FUNC(V3_Get)
{
	XMFLOAT3 *v = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	const lua_Integer pos = luaL_optinteger(vm, 2, 0);
	if (pos) {
		lua_pushnumber(vm, ((float *)&v->x)[pos]);
		return 1;
	} else {
		lua_pushnumber(vm, v->x);
		lua_pushnumber(vm, v->y);
		lua_pushnumber(vm, v->z);
		return 4;
	}
}

SIF_FUNC(V3_X)
{
	XMFLOAT3 *v = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	if (lua_isnumber(vm, 2)) {
		v->x = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, v->x);
		return 1;
	}
}

SIF_FUNC(V3_Y)
{
	XMFLOAT3 *v = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	if (lua_isnumber(vm, 2)) {
		v->y = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, v->y);
		return 1;
	}
}

SIF_FUNC(V3_Z)
{
	XMFLOAT3 *v = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	if (lua_isnumber(vm, 2)) {
		v->z = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, v->z);
		return 1;
	}
}

SIF_FUNC(V3_Fill)
{
	XMFLOAT3 *v = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	v->x = v->y = v->z = luaL_checknumber(vm, 2);
	return 0;
}

SIF_FUNC(V3_Zero)
{
	XMFLOAT3 *v = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	v->x = v->y = v->z = 0.f;
	return 0;
}

SIF_FUNC(V3_Add)
{
	XMFLOAT3 sv;
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3), *v2 = nullptr;

	if (luaL_testudata(vm, 2, SIF_NE_VEC3)) {
		v2 = (XMFLOAT3 *) lua_touserdata(vm, 2);
	} else {
		const float s = luaL_checknumber(vm, 2);
		sv = XMFLOAT3(s, s, s);
		v2 = &sv;
	}

	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	M_Store(v, XMVectorAdd(M_Load(v1), M_Load(v2)));

	return 1;
}

SIF_FUNC(V3_Sub)
{
	XMFLOAT3 sv;
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3), *v2 = nullptr;

	if (luaL_testudata(vm, 2, SIF_NE_VEC3)) {
		v2 = (XMFLOAT3 *) lua_touserdata(vm, 2);
	} else {
		const float s = luaL_checknumber(vm, 2);
		sv = XMFLOAT3(s, s, s);
		v2 = &sv;
	}

	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	M_Store(v, XMVectorSubtract(M_Load(v1), M_Load(v2)));

	return 1;
}

SIF_FUNC(V3_Mul)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);

	if (luaL_testudata(vm, 2, SIF_NE_VEC3)) {
		M_Store(v, XMVectorMultiply(M_Load(v1), M_Load((XMFLOAT3 *)lua_touserdata(vm, 2))));
	} else if (XMFLOAT4X4 *m = (XMFLOAT4X4 *)luaL_testudata(vm, 2, SIF_NE_MATRIX); m) {
		M_Store(v, XMVector3Transform(M_Load(v1), M_Load(m)));
	} else {
		const float s = luaL_checknumber(vm, 2);
		XMFLOAT3 sv(s, s, s);
		M_Store(v, XMVectorMultiply(M_Load(v1), M_Load(&sv)));
	}

	return 1;
}

SIF_FUNC(V3_Div)
{
	XMFLOAT3 sv;
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3), *v2 = nullptr;

	if (luaL_testudata(vm, 2, SIF_NE_VEC3)) {
		v2 = (XMFLOAT3 *) lua_touserdata(vm, 2);
	} else {
		const float s = luaL_checknumber(vm, 2);
		sv = XMFLOAT3(s, s, s);
		v2 = &sv;
	}

	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	M_Store(v, XMVectorDivide(M_Load(v1), M_Load(v2)));

	return 1;
}

SIF_FUNC(V3_MAdd)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	XMFLOAT3 *v2 = (XMFLOAT3 *)luaL_checkudata(vm, 2, SIF_NE_VEC3);
	XMFLOAT3 *v3 = (XMFLOAT3 *)luaL_checkudata(vm, 3, SIF_NE_VEC3);

	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	M_Store(v, XMVectorMultiplyAdd(M_Load(v1), M_Load(v2), M_Load(v3)));

	return 1;
}

SIF_FUNC(V3_Equal)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	XMFLOAT3 *v2 = (XMFLOAT3 *)luaL_checkudata(vm, 2, SIF_NE_VEC3);
	lua_pushboolean(vm, XMVector3Equal(M_Load(v1), M_Load(v2)));
	return 1;
}

SIF_FUNC(V3_Less)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	XMFLOAT3 *v2 = (XMFLOAT3 *)luaL_checkudata(vm, 2, SIF_NE_VEC3);
	lua_pushboolean(vm, XMVector3Less(M_Load(v1), M_Load(v2)));
	return 1;
}

SIF_FUNC(V3_LessEqual)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	XMFLOAT3 *v2 = (XMFLOAT3 *)luaL_checkudata(vm, 2, SIF_NE_VEC3);
	lua_pushboolean(vm, XMVector3LessOrEqual(M_Load(v1), M_Load(v2)));
	return 1;
}

SIF_FUNC(V3_Dot)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	XMFLOAT3 *v2 = (XMFLOAT3 *)luaL_checkudata(vm, 2, SIF_NE_VEC3);
	lua_pushnumber(vm, XMVectorGetX(XMVector3Dot(M_Load(v1), M_Load(v2))));
	return 1;
}

SIF_FUNC(V3_Cross)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	XMFLOAT3 *v2 = (XMFLOAT3 *)luaL_checkudata(vm, 2, SIF_NE_VEC3);

	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	M_Store(v, XMVector3Cross(M_Load(v1), M_Load(v2)));

	return 1;
}

SIF_FUNC(V3_Lerp)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	XMFLOAT3 *v2 = (XMFLOAT3 *)luaL_checkudata(vm, 2, SIF_NE_VEC3);

	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	M_Store(v, XMVectorLerp(M_Load(v1), M_Load(v2), luaL_checknumber(vm, 3)));

	return 1;
}

SIF_FUNC(V3_Distance)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	XMFLOAT3 *v2 = (XMFLOAT3 *)luaL_checkudata(vm, 2, SIF_NE_VEC3);
	lua_pushnumber(vm, XMVectorGetX(XMVector3Length(XMVectorSubtract(M_Load(v1), M_Load(v2)))));
	return 1;
}

SIF_FUNC(V3_Length)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	lua_pushnumber(vm, XMVectorGetX(XMVector3Length(M_Load(v1))));
	return 1;
}

SIF_FUNC(V3_LengthSquared)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	lua_pushnumber(vm, XMVectorGetX(XMVector3LengthSq(M_Load(v1))));
	return 1;
}

SIF_FUNC(V3_Normalize)
{
	XMFLOAT3 *v1 = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);

	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	M_Store(v, XMVector3Normalize(M_Load(v1)));

	return 1;
}

SIF_FUNC(V3_IsNaN)
{
	XMFLOAT3 *q = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	lua_pushboolean(vm, XMVector3IsNaN(M_Load(q)));
	return 1;
}

SIF_FUNC(V3_IsInfinite)
{
	XMFLOAT3 *q = (XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3);
	lua_pushboolean(vm, XMVector3IsInfinite(M_Load(q)));
	return 1;
}

void
SIface_PushVec3(lua_State *vm, const struct NeVec3 *v3)
{
	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	memcpy(v, v3, sizeof(*v));
}

// NeVec4

SIF_FUNC(V4_New)
{
	float x = (float)luaL_optnumber(vm, 1, 0.f);
	float y = (float)luaL_optnumber(vm, 2, 0.f);
	float z = (float)luaL_optnumber(vm, 3, 0.f);
	float w = (float)luaL_optnumber(vm, 4, 0.f);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);

	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;

	return 1;
}

SIF_FUNC(V4_Set)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);

	if (lua_isnumber(vm, 2)) {
		v->x = v->y = v->z = v->w = lua_tonumber(vm, 2);
	} else {
		XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_VEC4);
		memcpy(v, v2, sizeof(*v));
	}

	return 0;
}

SIF_FUNC(V4_Get)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	const lua_Integer pos = luaL_optinteger(vm, 2, 0);
	if (pos) {
		lua_pushnumber(vm, ((float *)&v->x)[pos]);
		return 1;
	} else {
		lua_pushnumber(vm, v->x);
		lua_pushnumber(vm, v->y);
		lua_pushnumber(vm, v->z);
		lua_pushnumber(vm, v->w);
		return 4;
	}
}

SIF_FUNC(V4_X)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	if (lua_isnumber(vm, 2)) {
		v->x = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, v->x);
		return 1;
	}
}

SIF_FUNC(V4_Y)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	if (lua_isnumber(vm, 2)) {
		v->y = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, v->y);
		return 1;
	}
}

SIF_FUNC(V4_Z)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	if (lua_isnumber(vm, 2)) {
		v->z = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, v->z);
		return 1;
	}
}

SIF_FUNC(V4_W)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	if (lua_isnumber(vm, 2)) {
		v->w = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, v->w);
		return 1;
	}
}

SIF_FUNC(V4_Fill)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	v->x = v->y = v->z = v->w = luaL_checknumber(vm, 2);
	return 0;
}

SIF_FUNC(V4_Zero)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	v->x = v->y = v->z = v->w = 0.f;
	return 0;
}

SIF_FUNC(V4_Add)
{
	XMFLOAT4 sv;
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4), *v2 = nullptr;

	if (luaL_testudata(vm, 2, SIF_NE_VEC4)) {
		v2 = (XMFLOAT4 *) lua_touserdata(vm, 2);
	} else {
		const float s = luaL_checknumber(vm, 2);
		sv = XMFLOAT4(s, s, s, s);
		v2 = &sv;
	}

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	M_Store(v, XMVectorAdd(M_Load(v1), M_Load(v2)));

	return 1;
}

SIF_FUNC(V4_Sub)
{
	XMFLOAT4 sv;
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4), *v2 = nullptr;

	if (luaL_testudata(vm, 2, SIF_NE_VEC4)) {
		v2 = (XMFLOAT4 *) lua_touserdata(vm, 2);
	} else {
		const float s = luaL_checknumber(vm, 2);
		sv = XMFLOAT4(s, s, s, s);
		v2 = &sv;
	}

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	M_Store(v, XMVectorSubtract(M_Load(v1), M_Load(v2)));

	return 1;
}

SIF_FUNC(V4_Mul)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);

	if (luaL_testudata(vm, 2, SIF_NE_VEC4))
		M_Store(v, XMVectorMultiply(M_Load(v1), M_Load((XMFLOAT4 *)lua_touserdata(vm, 2))));
	else if (XMFLOAT4X4 *m = (XMFLOAT4X4 *)luaL_testudata(vm, 2, SIF_NE_MATRIX); m)
		M_Store(v, XMVector4Transform(M_Load(v1), M_Load(m)));
	else
		M_Store(v, XMVectorMultiply(M_Load(v1), XMVectorReplicate(luaL_checknumber(vm, 2))));

	return 1;
}

SIF_FUNC(V4_Div)
{
	XMFLOAT4 sv;
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4), *v2 = nullptr;

	if (luaL_testudata(vm, 2, SIF_NE_VEC4)) {
		v2 = (XMFLOAT4 *) lua_touserdata(vm, 2);
	} else {
		const float s = luaL_checknumber(vm, 2);
		sv = XMFLOAT4(s, s, s, s);
		v2 = &sv;
	}

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	M_Store(v, XMVectorDivide(M_Load(v1), M_Load(v2)));

	return 1;
}

SIF_FUNC(V4_MAdd)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_VEC4);
	XMFLOAT4 *v3 = (XMFLOAT4 *)luaL_checkudata(vm, 3, SIF_NE_VEC4);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	M_Store(v, XMVectorMultiplyAdd(M_Load(v1), M_Load(v2), M_Load(v3)));

	return 1;
}

SIF_FUNC(V4_Equal)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_VEC4);
	lua_pushboolean(vm, XMVector4Equal(M_Load(v1), M_Load(v2)));
	return 1;
}

SIF_FUNC(V4_Less)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_VEC4);
	lua_pushboolean(vm, XMVector4Less(M_Load(v1), M_Load(v2)));
	return 1;
}

SIF_FUNC(V4_LessEqual)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_VEC4);
	lua_pushboolean(vm, XMVector4LessOrEqual(M_Load(v1), M_Load(v2)));
	return 1;
}

SIF_FUNC(V4_Dot)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_VEC4);
	lua_pushnumber(vm, XMVectorGetX(XMVector4Dot(M_Load(v1), M_Load(v2))));
	return 1;
}

SIF_FUNC(V4_Cross)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_VEC4);
	XMFLOAT4 *v3 = (XMFLOAT4 *)luaL_checkudata(vm, 3, SIF_NE_VEC4);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	M_Store(v, XMVector4Cross(M_Load(v1), M_Load(v2), M_Load(v3)));

	return 1;
}

SIF_FUNC(V4_Lerp)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_VEC4);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	M_Store(v, XMVectorLerp(M_Load(v1), M_Load(v2), luaL_checknumber(vm, 3)));

	return 1;
}

SIF_FUNC(V4_Distance)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_VEC4);
	lua_pushnumber(vm, XMVectorGetX(XMVector4Length(XMVectorSubtract(M_Load(v1), M_Load(v2)))));
	return 1;
}

SIF_FUNC(V4_Length)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	lua_pushnumber(vm, XMVectorGetX(XMVector4Length(M_Load(v1))));
	return 1;
}

SIF_FUNC(V4_LengthSquared)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	lua_pushnumber(vm, XMVectorGetX(XMVector4LengthSq(M_Load(v1))));
	return 1;
}

SIF_FUNC(V4_Normalize)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	M_Store(v, XMVector4Normalize(M_Load(v1)));

	return 1;
}

SIF_FUNC(V4_IsNaN)
{
	XMFLOAT4 *q = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	lua_pushboolean(vm, XMVector4IsNaN(M_Load(q)));
	return 1;
}

SIF_FUNC(V4_IsInfinite)
{
	XMFLOAT4 *q = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_VEC4);
	lua_pushboolean(vm, XMVector4IsInfinite(M_Load(q)));
	return 1;
}

void
SIface_PushVec4(lua_State *vm, const struct NeVec4 *v4)
{
	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	memcpy(v, v4, sizeof(*v));
}

// Quaternion

SIF_FUNC(Q_New)
{
	float x = (float)luaL_optnumber(vm, 1, 0.f);
	float y = (float)luaL_optnumber(vm, 2, 0.f);
	float z = (float)luaL_optnumber(vm, 3, 0.f);
	float w = (float)luaL_optnumber(vm, 4, 0.f);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);

	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;

	return 1;
}

SIF_FUNC(Q_Set)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);

	if (lua_isnumber(vm, 2)) {
		v->x = v->y = v->z = v->w = lua_tonumber(vm, 2);
	} else {
		XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_QUATERNION);
		memcpy(v, v2, sizeof(*v));
	}

	return 0;
}

SIF_FUNC(Q_Get)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	const lua_Integer pos = luaL_optinteger(vm, 2, 0);
	if (pos) {
		lua_pushnumber(vm, ((float *)&v->x)[pos]);
		return 1;
	} else {
		lua_pushnumber(vm, v->x);
		lua_pushnumber(vm, v->y);
		lua_pushnumber(vm, v->z);
		lua_pushnumber(vm, v->w);
		return 4;
	}
}

SIF_FUNC(Q_X)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	if (lua_isnumber(vm, 2)) {
		v->x = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, v->x);
		return 1;
	}
}

SIF_FUNC(Q_Y)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	if (lua_isnumber(vm, 2)) {
		v->y = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, v->y);
		return 1;
	}
}

SIF_FUNC(Q_Z)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	if (lua_isnumber(vm, 2)) {
		v->z = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, v->z);
		return 1;
	}
}

SIF_FUNC(Q_W)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	if (lua_isnumber(vm, 2)) {
		v->w = lua_tonumber(vm, 2);
		return 0;
	} else {
		lua_pushnumber(vm, v->w);
		return 1;
	}
}

SIF_FUNC(Q_Fill)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	v->x = v->y = v->z = v->w = luaL_checknumber(vm, 2);
	return 0;
}

SIF_FUNC(Q_Zero)
{
	XMFLOAT4 *v = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	v->x = v->y = v->z = v->w = 0.f;
	return 0;
}

SIF_FUNC(Q_Add)
{
	XMFLOAT4 sv;
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION), *v2 = nullptr;

	if (luaL_testudata(vm, 2, SIF_NE_QUATERNION)) {
		v2 = (XMFLOAT4 *) lua_touserdata(vm, 2);
	} else {
		const float s = luaL_checknumber(vm, 2);
		sv = XMFLOAT4(s, s, s, s);
		v2 = &sv;
	}

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);
	M_Store(v, XMVectorAdd(M_Load(v1), M_Load(v2)));

	return 1;
}

SIF_FUNC(Q_Sub)
{
	XMFLOAT4 sv;
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION), *v2 = nullptr;

	if (luaL_testudata(vm, 2, SIF_NE_QUATERNION)) {
		v2 = (XMFLOAT4 *) lua_touserdata(vm, 2);
	} else {
		const float s = luaL_checknumber(vm, 2);
		sv = XMFLOAT4(s, s, s, s);
		v2 = &sv;
	}

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);
	M_Store(v, XMVectorSubtract(M_Load(v1), M_Load(v2)));

	return 1;
}

SIF_FUNC(Q_Mul)
{
	XMFLOAT4 sv;
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION), *v2 = nullptr;

	if (luaL_testudata(vm, 2, SIF_NE_QUATERNION)) {
		v2 = (XMFLOAT4 *) lua_touserdata(vm, 2);
	} else {
		const float s = luaL_checknumber(vm, 2);
		sv = XMFLOAT4(s, s, s, s);
		v2 = &sv;
	}

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);
	M_Store(v, XMVectorMultiply(M_Load(v1), M_Load(v2)));

	return 1;
}

SIF_FUNC(Q_Div)
{
	XMFLOAT4 sv;
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION), *v2 = nullptr;

	if (luaL_testudata(vm, 2, SIF_NE_QUATERNION)) {
		v2 = (XMFLOAT4 *) lua_touserdata(vm, 2);
	} else {
		const float s = luaL_checknumber(vm, 2);
		sv = XMFLOAT4(s, s, s, s);
		v2 = &sv;
	}

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);
	M_Store(v, XMVectorDivide(M_Load(v1), M_Load(v2)));

	return 1;
}

SIF_FUNC(Q_MAdd)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_QUATERNION);
	XMFLOAT4 *v3 = (XMFLOAT4 *)luaL_checkudata(vm, 3, SIF_NE_QUATERNION);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);
	M_Store(v, XMVectorMultiplyAdd(M_Load(v1), M_Load(v2), M_Load(v3)));

	return 1;
}

SIF_FUNC(Q_Equal)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_QUATERNION);
	lua_pushboolean(vm, XMQuaternionEqual(M_Load(v1), M_Load(v2)));
	return 1;
}

SIF_FUNC(Q_Dot)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_QUATERNION);
	lua_pushnumber(vm, XMVectorGetX(XMQuaternionDot(M_Load(v1), M_Load(v2))));
	return 1;
}

SIF_FUNC(Q_Slerp)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_QUATERNION);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);
	M_Store(v, XMQuaternionSlerp(M_Load(v1), M_Load(v2), luaL_checknumber(vm, 3)));

	return 1;
}

SIF_FUNC(Q_Squad)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_QUATERNION);
	XMFLOAT4 *v3 = (XMFLOAT4 *)luaL_checkudata(vm, 3, SIF_NE_QUATERNION);
	XMFLOAT4 *v4 = (XMFLOAT4 *)luaL_checkudata(vm, 4, SIF_NE_QUATERNION);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);
	M_Store(v, XMQuaternionSquad(M_Load(v1), M_Load(v2), M_Load(v3), M_Load(v4), luaL_checknumber(vm, 3)));

	return 1;
}

SIF_FUNC(Q_Distance)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	XMFLOAT4 *v2 = (XMFLOAT4 *)luaL_checkudata(vm, 2, SIF_NE_QUATERNION);
	lua_pushnumber(vm, XMVectorGetX(XMQuaternionLength(XMVectorSubtract(M_Load(v1), M_Load(v2)))));
	return 1;
}

SIF_FUNC(Q_Length)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	lua_pushnumber(vm, XMVectorGetX(XMQuaternionLength(M_Load(v1))));
	return 1;
}

SIF_FUNC(Q_LengthSquared)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	lua_pushnumber(vm, XMVectorGetX(XMQuaternionLengthSq(M_Load(v1))));
	return 1;
}

SIF_FUNC(Q_Normalize)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);
	M_Store(v, XMQuaternionNormalize(M_Load(v1)));

	return 1;
}

SIF_FUNC(Q_ToAxisAngle)
{
	XMFLOAT4 *q = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);

	XMVECTOR axis;
	float angle;

	XMQuaternionToAxisAngle(&axis, &angle, M_Load(q));

	XMFLOAT3 *v = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	M_Store(v, axis);

	lua_pushnumber(vm, angle);

	return 2;
}

SIF_FUNC(Q_Conjugate)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);
	M_Store(v, XMQuaternionConjugate(M_Load(v1)));

	return 1;
}

SIF_FUNC(Q_Inverse)
{
	XMFLOAT4 *v1 = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);

	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);
	M_Store(v, XMQuaternionInverse(M_Load(v1)));

	return 1;
}

SIF_FUNC(Q_IsNaN)
{
	XMFLOAT4 *q = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	lua_pushboolean(vm, XMQuaternionIsNaN(M_Load(q)));
	return 1;
}

SIF_FUNC(Q_IsIdentity)
{
	XMFLOAT4 *q = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	lua_pushboolean(vm, XMQuaternionIsIdentity(M_Load(q)));
	return 1;
}

SIF_FUNC(Q_IsInfinite)
{
	XMFLOAT4 *q = (XMFLOAT4 *)luaL_checkudata(vm, 1, SIF_NE_QUATERNION);
	lua_pushboolean(vm, XMQuaternionIsInfinite(M_Load(q)));
	return 1;
}

void
SIface_PushQuaternion(lua_State *vm, const struct NeQuaternion *q)
{
	XMFLOAT4 *v = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_QUATERNION);
	memcpy(v, q, sizeof(*v));
}

// Matrix

SIF_FUNC(M4_New)
{
	XMFLOAT4X4 *m = (XMFLOAT4X4 *)lua_newuserdatauv(vm, sizeof(*m), 0);
	luaL_setmetatable(vm, SIF_NE_MATRIX);
	M_Store(m, XMMatrixIdentity());
	return 1;
}

/*SIF_FUNC(Set)
{
	XMFLOAT4X4 *m = (XMFLOAT4X4 *)luaL_checkudata(vm, 1, NE_MATRIX);
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	float m[16];
	for (int i = 0; i < 16; ++i)
		m[i] = luaL_checknumber(vm, i + 1);

	struct NeMatrix *v = (struct NeMatrix *)lua_touserdata(vm, 1);
	memcpy(v, m, sizeof(*v));

	lua_pushlightuserdata(vm, v);
	return 1;
}*/

SIF_FUNC(M4_Get)
{
	XMFLOAT4X4 *m = (XMFLOAT4X4 *)luaL_checkudata(vm, 1, SIF_NE_MATRIX);

	if (lua_isinteger(vm, 2) && lua_isinteger(vm, 3)) {
		const lua_Integer i = lua_tointeger(vm, 2);
		luaL_argcheck(vm, i >= 0 && i < 4, 2, "index out of range");

		const lua_Integer j = lua_tointeger(vm, 3);
		luaL_argcheck(vm, j >= 0 && j < 4, 2, "index out of range");

		lua_pushnumber(vm, m->m[i][j]);
	} else {
		const lua_Integer i = luaL_checkinteger(vm, 2);
		luaL_argcheck(vm, i >= 0 && i < 16, 2, "index out of range");

		const float *f = &m->_11;
		lua_pushnumber(vm, f[i]);
	}

	return 1;
}

SIF_FUNC(M4_Row)
{
	XMFLOAT4X4 *m = (XMFLOAT4X4 *)luaL_checkudata(vm, 1, SIF_NE_MATRIX);
	const lua_Integer idx = luaL_checkinteger(vm, 2);
	luaL_argcheck(vm, idx >= 0 && idx < 4, 2, "index out of range");

	XMFLOAT4 *r = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*r), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	memcpy(r, &m->m[idx], sizeof(*r));

	return 1;
}

SIF_FUNC(M4_Rows)
{
	XMFLOAT4X4 *m = (XMFLOAT4X4 *)luaL_checkudata(vm, 1, SIF_NE_MATRIX);

	XMFLOAT4 *r = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*r), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	memcpy(r, &m->m[0], sizeof(*r));

	r = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*r), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	memcpy(r, &m->m[1], sizeof(*r));

	r = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*r), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	memcpy(r, &m->m[2], sizeof(*r));

	r = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*r), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	memcpy(r, &m->m[3], sizeof(*r));

	return 4;
}

SIF_FUNC(M4_Mul)
{
	XMFLOAT4X4 *m = (XMFLOAT4X4 *)luaL_checkudata(vm, 1, SIF_NE_MATRIX);

	XMFLOAT4X4 *d = (XMFLOAT4X4 *)lua_newuserdatauv(vm, sizeof(*d), 0);
	luaL_setmetatable(vm, SIF_NE_MATRIX);

	if (XMFLOAT4X4 *m2 = (XMFLOAT4X4 *)luaL_testudata(vm, 2, SIF_NE_MATRIX); m2) {
		M_Store(d, XMMatrixMultiply(M_Load(m), M_Load(m2)));
	} else if (XMFLOAT4 *v4 = (XMFLOAT4 *)luaL_testudata(vm, 2, SIF_NE_VEC4); v4) {
	} else if (XMFLOAT3 *v3 = (XMFLOAT3 *)luaL_testudata(vm, 2, SIF_NE_VEC3); v3) {
	} else {
		const float f = luaL_checknumber(vm, 2);
		XMMATRIX sm = XMMatrixSet(f, f, f, f, f, f, f, f, f, f, f, f, f, f, f, f);
		M_Store(d, XMMatrixMultiply(M_Load(m), sm));
	}

	return 1;
}

SIF_FUNC(M4_SetIdentity)
{
	XMFLOAT4X4 *m = (XMFLOAT4X4 *)luaL_checkudata(vm, 1, SIF_NE_MATRIX);
	M_Store(m, XMMatrixIdentity());
	return 0;
}

SIF_FUNC(M4_Transpose)
{
	XMFLOAT4X4 *m = (XMFLOAT4X4 *)luaL_checkudata(vm, 1, SIF_NE_MATRIX);
	XMFLOAT4X4 *d = (XMFLOAT4X4 *)lua_newuserdatauv(vm, sizeof(*d), 0);
	luaL_setmetatable(vm, SIF_NE_MATRIX);
	M_Store(d, XMMatrixTranspose(M_Load(m)));
	return 1;
}

SIF_FUNC(M4_Inverse)
{
	XMFLOAT4X4 *m = (XMFLOAT4X4 *)luaL_checkudata(vm, 1, SIF_NE_MATRIX);
	XMFLOAT4X4 *d = (XMFLOAT4X4 *)lua_newuserdatauv(vm, sizeof(*d), 0);
	luaL_setmetatable(vm, SIF_NE_MATRIX);
	M_Store(d, XMMatrixInverse(NULL, M_Load(m)));
	return 1;
}

SIF_FUNC(M4_Determinant)
{
	lua_pushnumber(vm, XMVectorGetX(XMMatrixDeterminant(M_Load((XMFLOAT4X4 *)luaL_checkudata(vm, 1, SIF_NE_MATRIX)))));
	return 1;
}

SIF_FUNC(M4_Decompose)
{
	XMVECTOR sv, rv, tv;
	XMMatrixDecompose(&sv, &rv, &tv, M_Load((XMFLOAT4X4 *)luaL_checkudata(vm, 1, SIF_NE_MATRIX)));

	XMFLOAT3 *s = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*s), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	M_Store(s, sv);

	XMFLOAT4 *r = (XMFLOAT4 *)lua_newuserdatauv(vm, sizeof(*s), 0);
	luaL_setmetatable(vm, SIF_NE_VEC4);
	M_Store(r, rv);

	XMFLOAT3 *t = (XMFLOAT3 *)lua_newuserdatauv(vm, sizeof(*s), 0);
	luaL_setmetatable(vm, SIF_NE_VEC3);
	M_Store(t, tv);

	return 3;
}

SIF_FUNC(LookAt)
{
	const XMVECTOR eye = M_Load((XMFLOAT3 *)luaL_checkudata(vm, 1, SIF_NE_VEC3));
	const XMVECTOR front = M_Load((XMFLOAT3 *)luaL_checkudata(vm, 2, SIF_NE_VEC3));
	const XMVECTOR up = M_Load((XMFLOAT3 *)luaL_checkudata(vm, 3, SIF_NE_VEC3));

	XMFLOAT4X4 *m = (XMFLOAT4X4 *)lua_newuserdatauv(vm, sizeof(*m), 0);
	luaL_setmetatable(vm, SIF_NE_MATRIX);
	M_Store(m, XMMatrixLookAtRH(eye, front, up));

	return 1;
}

SIF_FUNC(Scale)
{
	float x, y, z;

	if (XMFLOAT3 *v = (XMFLOAT3 *)luaL_testudata(vm, 1, SIF_NE_VEC3); v) {
		x = v->x; y = v->z; z = v->z;
	} else {
		x = luaL_checknumber(vm, 1);
		y = luaL_checknumber(vm, 2);
		z = luaL_checknumber(vm, 3);
	}

	XMFLOAT4X4 *m = (XMFLOAT4X4 *)lua_newuserdatauv(vm, sizeof(*m), 0);
	luaL_setmetatable(vm, SIF_NE_MATRIX);
	M_Store(m, XMMatrixScaling(x, y, z));

	return 1;
}

SIF_FUNC(Translation)
{
	float x, y, z;

	if (XMFLOAT3 *v = (XMFLOAT3 *)luaL_testudata(vm, 1, SIF_NE_VEC3); v) {
		x = v->x; y = v->z; z = v->z;
	} else {
		x = luaL_checknumber(vm, 1);
		y = luaL_checknumber(vm, 2);
		z = luaL_checknumber(vm, 3);
	}

	XMFLOAT4X4 *m = (XMFLOAT4X4 *)lua_newuserdatauv(vm, sizeof(*m), 0);
	luaL_setmetatable(vm, SIF_NE_MATRIX);
	M_Store(m, XMMatrixTranslation(x, y, z));

	return 1;
}

SIF_FUNC(Rotation)
{
	float x, y, z;

	if (XMFLOAT3 *v = (XMFLOAT3 *)luaL_testudata(vm, 1, SIF_NE_VEC3); v) {
		x = v->x; y = v->z; z = v->z;
	} else {
		x = luaL_checknumber(vm, 1);
		y = luaL_checknumber(vm, 2);
		z = luaL_checknumber(vm, 3);
	}

	XMFLOAT4X4 *m = (XMFLOAT4X4 *)lua_newuserdatauv(vm, sizeof(*m), 0);
	luaL_setmetatable(vm, SIF_NE_MATRIX);
	M_Store(m, XMMatrixRotationRollPitchYaw(x, y, z));

	return 1;
}

SIF_FUNC(Perspective)
{
	XMFLOAT4X4 *m = (XMFLOAT4X4 *)lua_newuserdatauv(vm, sizeof(*m), 0);
	luaL_setmetatable(vm, SIF_NE_MATRIX);
	M_Store(m, XMMatrixPerspectiveFovRH(luaL_checknumber(vm, 1), luaL_checknumber(vm, 2), luaL_checknumber(vm, 3), luaL_checknumber(vm, 4)));
	return 1;
}

SIF_FUNC(Orthographic)
{
	XMFLOAT4X4 *m = (XMFLOAT4X4 *)lua_newuserdatauv(vm, sizeof(*m), 0);
	luaL_setmetatable(vm, SIF_NE_MATRIX);
	M_Store(m, XMMatrixOrthographicRH(luaL_checknumber(vm, 1), luaL_checknumber(vm, 2), luaL_checknumber(vm, 3), luaL_checknumber(vm, 4)));
	return 1;
}

void
SIface_PushMatrix(lua_State *vm, const struct NeMatrix *m)
{
	XMFLOAT4X4 *v = (XMFLOAT4X4 *)lua_newuserdatauv(vm, sizeof(*v), 0);
	luaL_setmetatable(vm, SIF_NE_MATRIX);
	memcpy(v, m, sizeof(*v));
}

// Shared

SIF_FUNC(ToString)
{
	if (XMFLOAT4 *v4 = (XMFLOAT4 *)luaL_testudata(vm, 1, SIF_NE_VEC4); v4) {
		lua_pushfstring(vm, "NeVec4(%f, %f, %f, %f)", v4->x, v4->y, v4->z, v4->w);
		return 1;
	}

	if (XMFLOAT4 *q = (XMFLOAT4 *)luaL_testudata(vm, 1, SIF_NE_QUATERNION); q) {
		lua_pushfstring(vm, "NeQuaternion(%f, %f, %f, %f)", q->x, q->y, q->z, q->w);
		return 1;
	}

	if (XMFLOAT3 *v3 = (XMFLOAT3 *)luaL_testudata(vm, 1, SIF_NE_VEC3); v3) {
		lua_pushfstring(vm, "NeVec3(%f, %f, %f)", v3->x, v3->y, v3->z);
		return 1;
	}

	if (XMFLOAT4X4 *m = (XMFLOAT4X4 *)luaL_testudata(vm, 1, SIF_NE_MATRIX); m) {
		lua_pushfstring(vm, "NeMatrix(\n\t%f, %f, %f, %f\n\t%f, %f, %f, %f\n\t%f, %f, %f, %f\n\t%f, %f, %f, F5\n)",
						m->_11, m->_12, m->_13, m->_14,
						m->_21, m->_22, m->_23, m->_24,
						m->_31, m->_32, m->_33, m->_34,
						m->_41, m->_42, m->_43, m->_44);
		return 1;
	}

	return 0;
}

static void
RegisterVec3(lua_State *vm)
{
	luaL_Reg meta[] = {
		{ "__index",    NULL },
		{ "__add",      Sif_V3_Add },
		{ "__sub",      Sif_V3_Sub },
		{ "__mul",      Sif_V3_Mul },
		{ "__div",      Sif_V3_Div },
		{ "__eq",       Sif_V3_Equal },
		{ "__lt",       Sif_V3_Less },
		{ "__le",       Sif_V3_LessEqual },
		{ "__tostring", Sif_ToString },
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		{ "x",          Sif_V3_X },
		{ "y",          Sif_V3_Y },
		{ "z",          Sif_V3_Z },
		{ "Set",        Sif_V3_Set },
		{ "Get",        Sif_V3_Get },
		{ "MAdd",       Sif_V3_MAdd },
		{ "Dot",        Sif_V3_Dot },
		{ "Cross",      Sif_V3_Cross },
		{ "Lerp",       Sif_V3_Lerp },
		{ "Length",     Sif_V3_Length },
		{ "LengthSq",   Sif_V3_LengthSquared },
		{ "Normalize",  Sif_V3_Normalize },
		{ "Distance",   Sif_V3_Distance },
		{ "Zero",       Sif_V3_Zero },
		{ "Fill",       Sif_V3_Fill },
		{ "IsNaN",      Sif_V3_IsNaN },
		{ "IsInfinite", Sif_V3_IsInfinite },
		SIF_ENDREG()
	};

	lua_register(vm, SIF_NE_VEC3, Sif_V3_New);
	luaL_newmetatable(vm, SIF_NE_VEC3);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);
};

static void
RegisterVec4(lua_State *vm)
{
	luaL_Reg meta[] = {
		{ "__index",    NULL },
		{ "__add",      Sif_V4_Add },
		{ "__sub",      Sif_V4_Sub },
		{ "__mul",      Sif_V4_Mul },
		{ "__div",      Sif_V4_Div },
		{ "__eq",       Sif_V4_Equal },
		{ "__lt",       Sif_V4_Less },
		{ "__le",       Sif_V4_LessEqual },
		{ "__tostring", Sif_ToString },
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		{ "x",          Sif_V4_X },
		{ "y",          Sif_V4_Y },
		{ "z",          Sif_V4_Z },
		{ "w",          Sif_V4_W },
		{ "Set",        Sif_V4_Set },
		{ "Get",        Sif_V4_Get },
		{ "MAdd",       Sif_V4_MAdd },
		{ "Dot",        Sif_V4_Dot },
		{ "Cross",      Sif_V4_Cross },
		{ "Lerp",       Sif_V4_Lerp },
		{ "Length",     Sif_V4_Length },
		{ "LengthSq",   Sif_V4_LengthSquared },
		{ "Normalize",  Sif_V4_Normalize },
		{ "Distance",   Sif_V4_Distance },
		{ "Zero",       Sif_V4_Zero },
		{ "Fill",       Sif_V4_Fill },
		{ "IsNaN",      Sif_V4_IsNaN },
		{ "IsInfinite", Sif_V4_IsInfinite },
		SIF_ENDREG()
	};

	lua_register(vm, SIF_NE_VEC4, Sif_V4_New);
	luaL_newmetatable(vm, SIF_NE_VEC4);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);
};

static void
RegisterQuat(lua_State *vm)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		{ "__add",       Sif_Q_Add },
		{ "__sub",       Sif_Q_Sub },
		{ "__mul",       Sif_Q_Mul },
		{ "__div",       Sif_Q_Div },
		{ "__eq",        Sif_Q_Equal },
		{ "__tostring",  Sif_ToString },
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		{ "x",           Sif_Q_X },
		{ "y",           Sif_Q_Y },
		{ "z",           Sif_Q_Z },
		{ "w",           Sif_Q_W },
		{ "Set",         Sif_Q_Set },
		{ "Get",         Sif_Q_Get },
		{ "MAdd",        Sif_Q_MAdd },
		{ "Dot",         Sif_Q_Dot },
		{ "Slerp",       Sif_Q_Slerp },
		{ "Squad",       Sif_Q_Squad },
		{ "ToAxisAngle", Sif_Q_ToAxisAngle },
		{ "Conjugate",   Sif_Q_Conjugate },
		{ "Inverse",     Sif_Q_Inverse },
		{ "Length",      Sif_Q_Length },
		{ "LengthSq",    Sif_Q_LengthSquared },
		{ "Normalize",   Sif_Q_Normalize },
		{ "Distance",    Sif_Q_Distance },
		{ "Zero",        Sif_Q_Zero },
		{ "Fill",        Sif_Q_Fill },
		{ "IsNaN",       Sif_Q_IsNaN },
		{ "IsIdentity",  Sif_Q_IsIdentity },
		{ "IsInfinite",  Sif_Q_IsInfinite },
		SIF_ENDREG()
	};

	lua_register(vm, SIF_NE_QUATERNION, Sif_Q_New);
	luaL_newmetatable(vm, SIF_NE_QUATERNION);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);
};

static void
RegisterMatrix(lua_State *vm)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		{ "__mul",       Sif_M4_Mul },
		{ "__tostring",  Sif_ToString },
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		{ "Inverse",     Sif_M4_Inverse },
		{ "Transpose",   Sif_M4_Transpose },
		{ "SetIdentity", Sif_M4_SetIdentity },
		{ "Determinant", Sif_M4_Determinant },
		{ "Decompose",   Sif_M4_Decompose },
		{ "Rows",        Sif_M4_Rows },
		{ "Row",         Sif_M4_Row },
		{ "Get",         Sif_M4_Get },
		SIF_ENDREG()
	};

	lua_register(vm, SIF_NE_MATRIX, Sif_M4_New);
	luaL_newmetatable(vm, SIF_NE_MATRIX);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);
};

NE_ENGINE_IF_MOD(Math)
{
	RegisterVec4(vm);
	RegisterVec3(vm);
	RegisterQuat(vm);
	RegisterMatrix(vm);

	luaL_Reg reg[] =
	{
		SIF_REG(LookAt),
		SIF_REG(Scale),
		SIF_REG(Translation),
		SIF_REG(Rotation),
		SIF_REG(Perspective),
		SIF_REG(Orthographic),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Matrix");
}

/* NekoEngine
 *
 * l_Vec4.c
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
