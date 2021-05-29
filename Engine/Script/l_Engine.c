#include <Script/Script.h>
#include <Engine/Engine.h>
#include <Engine/Version.h>

#include "Interface.h"

SIF_LUSRDATA(Screen, E_screen);
SIF_NUMBER(ScreenWidth, *E_screenWidth);
SIF_NUMBER(ScreenHeight, *E_screenHeight);
SIF_NUMBER(DeltaTime, E_deltaTime);
SIF_NUMBER(Time, E_Time());
SIF_STRING(Version, E_VER_STR_A);
SIF_INTEGER(ScriptAPI, SCRIPT_API_VERSION);

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
		SIF_REG(Version),
		SIF_REG(ScriptAPI),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "E");
}
