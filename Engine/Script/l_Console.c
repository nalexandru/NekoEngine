#include <Script/Script.h>
#include <Engine/Console.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

SIF_FUNC(Puts)
{
	const char *str = luaL_checkstring(vm, 1);
	if (str)
		E_ConsolePuts(str);
	return 0;
}

void
SIface_OpenConsole(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(Puts),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "C");
}
