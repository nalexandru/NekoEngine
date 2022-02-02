#include <Script/Script.h>
#include <Engine/Console.h>
#include <Engine/Resource.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

SIF_FUNC(Load)
{
	NeHandle res = E_LoadResource(luaL_checkstring(vm, 1), luaL_checkstring(vm, 2));
	if (res)
		lua_pushlightuserdata(vm, (void *)res);
	else
		lua_pushnil(vm);
	return 1;
}

SIF_FUNC(Ptr)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushlightuserdata(vm, E_ResourcePtr((NeHandle)lua_touserdata(vm, 1)));
	return 1;
}

SIF_FUNC(References)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushinteger(vm, E_ResourceReferences((NeHandle)lua_touserdata(vm, 1)));
	return 1;
}

SIF_FUNC(Retain)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	E_RetainResource((NeHandle)lua_touserdata(vm, 1));
	return 0;
}

SIF_FUNC(Release)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	E_ReleaseResource((NeHandle)lua_touserdata(vm, 1));
	return 0;
}

SIF_FUNC(ToGPU)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushinteger(vm, E_ResHandleToGPU((NeHandle)lua_touserdata(vm, 1)));
	return 1;
}

SIF_FUNC(FromGPU)
{
	lua_pushlightuserdata(vm, (void *)E_GPUHandleToRes((uint16_t)luaL_checkinteger(vm, 1), luaL_checkstring(vm, 2)));
	return 1;
}

SIF_FUNC(Unload)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	E_UnloadResource((NeHandle)lua_touserdata(vm, 1));
	return 0;
}

void
SIface_OpenResource(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(ToGPU),
		SIF_REG(FromGPU),

		SIF_REG(Retain),
		SIF_REG(Release),
		SIF_REG(References),

		SIF_REG(Ptr),

		SIF_REG(Load),
		SIF_REG(Unload),

		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Resource");
}
