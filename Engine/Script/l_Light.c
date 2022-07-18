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

	M_CopyVec3(&l->color, &new);
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
