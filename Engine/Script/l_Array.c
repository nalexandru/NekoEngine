#include <Script/Interface.h>
#include <Runtime/Runtime.h>

#define NE_ARRAY	"NeArray *"

SIF_FUNC(Get)
{
	return 0;
}

SIF_FUNC(Add)
{
	return 0;
	//return Rt_ArrayAdd(luaL_checkudata(vm, 1, NE_ARRAY), lua_touserdata())
}

SIF_FUNC(Insert)
{
	return 0;
}

// clone, copy

SIF_FUNC(Resize)
{
	return 0;
}

SIF_FUNC(Allocate)
{
	return 0;
}

SIF_FUNC(Find)
{
	return 0;
}

SIF_FUNC(BSearch)
{
	return 0;
}

SIF_FUNC(Sort)
{
	return 0;
}

SIF_FUNC(Reverse)
{
	return 0;
}

SIF_FUNC(Fill)
{
	return 0;
}

SIF_FUNC(Zero)
{
	return 0;
}

SIF_FUNC(Clear)
{
	return 0;
}

SIF_FUNC(Term)
{
	return 0;
}

NE_SCRIPT_INTEFACE(NeArray)
{
	luaL_Reg arrayMeta[] = {
		{"__index", NULL},
		{"__gc", Sif_Term},
		SIF_ENDREG()
	};

	luaL_Reg arrayMethods[] = {
		SIF_REG(Get),
		SIF_REG(Add),
		SIF_REG(Insert),
		SIF_REG(Find),
		SIF_REG(Allocate),
		SIF_REG(BSearch),
		SIF_REG(Resize),
		SIF_REG(Sort),
		SIF_REG(Reverse),
		SIF_REG(Fill),
		SIF_REG(Zero),
		SIF_REG(Clear),
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, NE_ARRAY);
	luaL_setfuncs(vm, arrayMeta, 0);
	luaL_newlibtable(vm, arrayMethods);
	luaL_setfuncs(vm, arrayMethods, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	/*luaL_Reg reg[] =
	{
		SIF_REG(SetValue),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Array");*/

	return 1;
}

/* NekoEngine
 *
 * l_Array.c
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
