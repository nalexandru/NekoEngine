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

/* NekoEngine
 *
 * l_Config.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
