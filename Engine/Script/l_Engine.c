#include <Engine/Engine.h>
#include <Script/Script.h>

#include "Interface.h"

SIF_LUSRDATA(Screen, E_screen);
SIF_NUMBER(ScreenWidth, *E_screenWidth);
SIF_NUMBER(ScreenHeight, *E_screenHeight);
SIF_NUMBER(DeltaTime, E_deltaTime);
SIF_NUMBER(Time, E_Time());

void
SIface_OpenEngine(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(Screen),
		SIF_REG(ScreenWidth),
		SIF_REG(ScreenHeight),
		SIF_REG(DeltaTime),
		SIF_REG(Time),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "E");
}
