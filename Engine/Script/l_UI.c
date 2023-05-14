#include <UI/UI.h>
#include <Scene/Components.h>
#include <Script/Interface.h>

SIF_FUNC(Text)
{
	SIF_CHECKCOMPONENT(1, ctx, NE_UI_CONTEXT, struct NeUIContext *);

	struct NeFont *f = lua_touserdata(vm, 6);
	UI_DrawText(ctx, luaL_checkstring(vm, 2),
		luaL_checknumber(vm, 3), luaL_checknumber(vm, 4), luaL_checknumber(vm, 5), f);

	return 0;
}

SIF_FUNC(__tostring)
{
	lua_pushliteral(vm, "NeUIContext");
	return 1;
}

NE_SCRIPT_INTEFACE(NeUI)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(Text),
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, NE_UI_CONTEXT);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	return 1;
}

/* NekoEngine
 *
 * l_UI.c
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
