#include <Engine/Console.h>
#include <Engine/Resource.h>
#include <Script/Interface.h>

#include "EngineInterface.h"

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

NE_ENGINE_IF_MOD(Resource)
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

/* NekoEngine
 *
 * l_Resource.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
