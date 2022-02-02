#include <Script/Script.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

SIF_FUNC(Get)
{
	const char *str = luaL_checkstring(vm, 1);
	const struct NeCVar *var = E_GetCVar(str);

	if (var) {
		switch (var->type) {
		case CV_Int32: lua_pushinteger(vm, var->i32); break;
		case CV_UInt32: lua_pushinteger(vm, var->u32); break;
		case CV_UInt64: lua_pushinteger(vm, var->u64); break;
		case CV_Float: lua_pushnumber(vm, var->flt); break;
		case CV_Bool: lua_pushboolean(vm, var->bln); break;
		case CV_String: lua_pushstring(vm, var->str); break;
		}
	} else {
		lua_pushnil(vm);
	}

	return 1;
}

SIF_FUNC(Set)
{
	const char *str = luaL_checkstring(vm, 1);
	struct NeCVar *var = E_GetCVar(str);

	if (var) {
		switch (var->type) {
		case CV_Int32: var->i32 = (int32_t)luaL_checkinteger(vm, 2); break;
		case CV_UInt32: var->u32 = (uint32_t)luaL_checkinteger(vm, 2); break;
		case CV_UInt64: var->u64 = luaL_checkinteger(vm, 2); break;
		case CV_Float: var->flt = (float)luaL_checknumber(vm, 2); break;
		case CV_String: E_SetCVarStr(str, luaL_checkstring(vm, 2)); break;
		case CV_Bool: {
			if (!lua_isboolean(vm, 2))
				luaL_argerror(vm, 2, "Must be a boolean");

			var->bln = lua_toboolean(vm, 2);
		} break;
		}
	}

	return 0;
}

void
SIface_OpenConfig(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(Get),
		SIF_REG(Set),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Config");
}
