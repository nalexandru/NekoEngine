#include <Engine/Event.h>
#include <Script/Script.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

SIF_FUNC(Broadcast)
{
	E_Broadcast(luaL_checkstring(vm, 1), NULL);
	return 0;
}

SIF_FUNC(Register)
{
//	const char *name = luaL_checkstring(vm, 1);
	// to function
	luaL_error(vm, "not implemented");
	return 0;
}

SIF_FUNC(Unregister)
{
	E_UnregisterHandler(luaL_checkinteger(vm, 1));
	return 0;
}

void
SIface_OpenEvent(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(Broadcast),
		SIF_REG(Register),
		SIF_REG(Unregister),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Event");
}
