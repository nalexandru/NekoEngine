#include <UI/UI.h>
#include <Script/Script.h>
#include <Engine/Console.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

SIF_FUNC(DrawText)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	struct NeFont *f = NULL;
	if (lua_islightuserdata(vm, 6))
		f = lua_touserdata(vm, 6);
	else if (!lua_isnil(vm, 6))
		luaL_argerror(vm, 6, "");

	UI_DrawText(lua_touserdata(vm, 1), luaL_checkstring(vm, 2),
		(float)luaL_checknumber(vm, 3), (float)luaL_checknumber(vm, 4), (float)luaL_checknumber(vm, 5), f);

	return 0;
}

void
SIface_OpenUI(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(DrawText),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "UI");
}
