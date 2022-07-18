#include <Script/Script.h>
#include <Scene/Camera.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

SIF_FUNC(GetRotation)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeCamera *cam = lua_touserdata(vm, 1);

	lua_pushnumber(vm, cam->rotation.x);
	lua_pushnumber(vm, cam->rotation.y);
	lua_pushnumber(vm, cam->rotation.z);

	return 3;
}

SIF_FUNC(SetRotation)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeVec3 new =
	{
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	};

	struct NeCamera *cam = lua_touserdata(vm, 1);

	M_CopyVec3(&cam->rotation, &new);
	return 0;
}

SIF_FUNC(GetFOV)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");
	struct NeCamera *cam = lua_touserdata(vm, 1);
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
	struct NeCamera *cam = lua_touserdata(vm, 1);
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
	struct NeCamera *cam = lua_touserdata(vm, 1);
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
	struct NeCamera *cam = lua_touserdata(vm, 1);
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
