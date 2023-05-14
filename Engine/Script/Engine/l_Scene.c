#include <Scene/Scene.h>
#include <Engine/Entity.h>
#include <Script/Interface.h>

#include "EngineInterface.h"

SIF_FUNC(ActiveScene)
{
	Sc_PushScriptWrapper(vm, Scn_activeScene, SIF_NE_SCENE);
	return 1;
}

SIF_FUNC(GetScene)
{
	Sc_PushScriptWrapper(vm, Scn_GetScene((uint8_t)luaL_checkinteger(vm, 1)), SIF_NE_SCENE);
	return 1;
}

SIF_FUNC(Create)
{
	Sc_PushScriptWrapper(vm, Scn_CreateScene(luaL_checkstring(vm, 1)), SIF_NE_SCENE);
	return 1;
}

SIF_FUNC(AddEntity)
{
	SIF_TESTCOMPONENT(1, scn, SIF_NE_SCENE, struct NeScene *);
	const char *name = luaL_checkstring(vm, 2), *type = luaL_checkstring(vm, 3);

	NeEntityHandle ent = E_CreateEntityS(scn, name, type);
	if (!ent)
		luaL_error(vm, "Failed to create entity");

	Sc_PushScriptWrapper(vm, ent, SIF_NE_ENTITY);
	return 1;
}

SIF_FUNC(Activate)
{
	SIF_TESTCOMPONENT(1, scn, SIF_NE_SCENE, struct NeScene *);
	Scn_ActivateScene(scn);
	return 0;
}

SIF_FUNC(FindEntity)
{
	SIF_TESTCOMPONENT(1, scn, SIF_NE_SCENE, struct NeScene *);
	void *p = E_FindEntityS(scn, luaL_checkstring(vm, 2));
	if (p)
		Sc_PushScriptWrapper(vm, p, SIF_NE_ENTITY);
	else
		lua_pushnil(vm);
	return 1;
}

SIF_FUNC(CreateTerrain)
{
	SIF_TESTCOMPONENT(1, scn, SIF_NE_SCENE, struct NeScene *);

	if (!lua_istable(vm, 2))
		luaL_argerror(vm, 2, "Must be a table");

	const struct NeTerrainCreateInfo tci =
	{
		.tileSize = SIF_INTFIELD(2, "tileSize"),
		.tileCount = SIF_INTFIELD(2, "tileCount"),
		.maxHeight = SIF_FLOATFIELD(2, "maxHeight"),
		.material = (NeHandle)SIF_LUSRDATAFIELD(2, "material"),
		.mapFile = (char *)SIF_STRINGFIELD(2, "mapFile")
	};

	lua_pushboolean(vm, Scn_CreateTerrain(scn, &tci));
	return 1;
}

SIF_FUNC(__tostring)
{
	lua_pushliteral(vm, "NeScene");
	return 1;
}

NE_ENGINE_IF_MOD(Scene)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(AddEntity),
		SIF_REG(FindEntity),
		SIF_REG(Activate),
		SIF_REG(CreateTerrain),
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, SIF_NE_SCENE);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	luaL_Reg reg[] =
	{
		SIF_REG(ActiveScene),
		SIF_REG(GetScene),
		SIF_REG(Create),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Scene");
}

/* NekoEngine
 *
 * l_Scene.c
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
