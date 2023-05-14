#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <Engine/IO.h>
#include <Engine/Config.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Script/Interface.h>

#define SCRIPTMOD	"Script"

static const luaL_Reg f_luaLibs[] = {
	// Core Lua libs
	{ LUA_GNAME, luaopen_base },
	{ LUA_LOADLIBNAME, luaopen_package },
	{ LUA_COLIBNAME, luaopen_coroutine },
	{ LUA_TABLIBNAME, luaopen_table },
	{ LUA_STRLIBNAME, luaopen_string },
	{ LUA_MATHLIBNAME, luaopen_math },
	{ LUA_UTF8LIBNAME, luaopen_utf8 },
	{ LUA_DBLIBNAME, luaopen_debug },

	{ NULL, NULL }
};

struct ScriptInterface
{
	uint64_t hash;
	lua_CFunction open;
	char *script;
	size_t scriptLen;
};

static struct NeArray f_scriptIfaces;
static void *LuaAlloc(void *ud, void *ptr, size_t oSize, size_t nSize);
static int LuaSearcher(lua_State *vm);

lua_State *
Sc_CreateVM(void)
{
	lua_State *vm;

	vm = luaL_newstate_alloc(LuaAlloc);
	if (!vm)
		return NULL;

	lua_setallocf(vm, LuaAlloc, NULL);

	for (const luaL_Reg *lib = f_luaLibs; lib->func; ++lib) {
		luaL_requiref(vm, lib->name, lib->func, 1);
		lua_pop(vm, 1);
	}

	luaL_requiref(vm, "NeScriptComponent", SIface_ScriptComponent, 0);
	lua_pop(vm, 1);

	lua_getglobal(vm, "package");
	lua_getfield(vm, -1, "searchers");

	const lua_Unsigned len = lua_rawlen(vm, -1);

	lua_pushcfunction(vm, LuaSearcher);
	lua_rawseti(vm, -2, len + 1);

	lua_pop(vm, 2);

	return vm;
}

const char *
Sc_Execute(lua_State *vm, const char *command)
{
	const char *rc = NULL;
	if (luaL_dostring(vm, command) && lua_gettop(vm)) {
		rc = Rt_TransientStrDup(lua_tostring(vm, -1));
		lua_pop(vm, 1);
	}
	return rc;
}

const char *
Sc_ExecuteFile(lua_State *vm, const char *path)
{
	int64_t len;
	char *source;
	struct NeStream stm;

	if (!E_FileStream(path, IO_READ, &stm))
		return "File not found";

	len = E_StreamLength(&stm);
	source = Sys_Alloc((size_t)len, 1, MH_Transient);

	E_ReadStream(&stm, source, E_StreamLength(&stm));
	E_CloseStream(&stm);

	return Sc_Execute(vm, source);
}

bool
Sc_LoadScript(lua_State *vm, const char *source)
{
	if (luaL_dostring(vm, source) && lua_gettop(vm)) {
		Sys_LogEntry(SCRIPTMOD, LOG_CRITICAL, "Failed to load script: %s", lua_tostring(vm, -1));
		lua_pop(vm, 1);
		return false;
	}

	return true;
}

bool
Sc_LoadScriptFile(lua_State *vm, const char *path)
{
	struct NeStream stm;
	if (!E_FileStream(path, IO_READ, &stm))
		return false;

	const bool rc = Sc_LoadScriptStream(vm, &stm);

	E_CloseStream(&stm);
	return rc;
}

bool
Sc_LoadScriptStream(lua_State *vm, struct NeStream *stm)
{
	const int64_t len = E_StreamLength(stm) + 1;
	char *source = Sys_Alloc((size_t)len, 1, MH_Transient);

	E_ReadStream(stm, source, E_StreamLength(stm));

	source[len] = 0x0;

	if (luaL_dostring(vm, source) && lua_gettop(vm)) {
		Sys_LogEntry(SCRIPTMOD, LOG_CRITICAL, "Failed to load script: %s", lua_tostring(vm, -1));
		lua_pop(vm, 1);
		return false;
	}

	return true;
}

void
Sc_LogStackDump(lua_State *vm, int severity)
{
	int top = lua_gettop(vm);

	Sys_LogEntry(SCRIPTMOD, severity, "Lua stack dump:");
	for (int i = 0; i <= top; ++i) {
		switch (lua_type(vm, i)) {
		case LUA_TSTRING:
			Sys_LogEntry(SCRIPTMOD, severity, "\t%d: <string> [%s]", i, lua_tostring(vm, i));
		break;
		case LUA_TBOOLEAN:
			Sys_LogEntry(SCRIPTMOD, severity, "\t%d: <boolean> [%s]", i, lua_toboolean(vm, i) ? "true" : "false");
		break;
		case LUA_TFUNCTION:
			Sys_LogEntry(SCRIPTMOD, severity, "\t%d: <function> [%p]", i, lua_tocfunction(vm, i));
		break;
		case LUA_TUSERDATA:
			Sys_LogEntry(SCRIPTMOD, severity, "\t%d: <user data> [%p]", i, lua_touserdata(vm, i));
		break;
		case LUA_TLIGHTUSERDATA:
			Sys_LogEntry(SCRIPTMOD, severity, "\t%d: <light user data> [%p]", i, lua_touserdata(vm, i));
		break;
		case LUA_TNUMBER:
			Sys_LogEntry(SCRIPTMOD, severity, "\t%d: <number> [%q]", i, lua_tonumber(vm, i));
		break;
		default:
			Sys_LogEntry(SCRIPTMOD, severity, "\t%d: <%s>", i, lua_typename(vm, i));
		break;
		}
	}
	Sys_LogEntry(SCRIPTMOD, severity, "End");
}

bool
Sc_RegisterInterface(const char *name, lua_CFunction open)
{
	if (!f_scriptIfaces.data)
		if (!Rt_InitArray(&f_scriptIfaces, 10, sizeof(struct ScriptInterface), MH_System))
			return false;

	struct ScriptInterface si =
	{
		.hash = Rt_HashString(name),
		.open = open
	};
	return Rt_ArrayAdd(&f_scriptIfaces, &si);
}

bool
Sc_RegisterInterfaceScript(const char *name, const char *script)
{
	if (!f_scriptIfaces.data)
		if (!Rt_InitArray(&f_scriptIfaces, 10, sizeof(struct ScriptInterface), MH_System))
			return false;

	struct ScriptInterface si =
	{
		.hash = Rt_HashString(name),
		.script = Rt_StrDup(script, MH_System),
		.scriptLen = strlen(script)
	};
	return Rt_ArrayAdd(&f_scriptIfaces, &si);
}

void
Sc_DestroyVM(lua_State *vm)
{
	if (vm)
		lua_close(vm);
}

bool
Sc_InitScriptSystem(void)
{
	if (!f_scriptIfaces.data)
		if (!Rt_InitArray(&f_scriptIfaces, 10, sizeof(struct ScriptInterface), MH_System))
			return false;

#ifdef _DEBUG
	E_GetCVarBln("Script_CheckComponentFieldAccess", true);
#endif

	return true;
}

void
Sc_TermScriptSystem(void)
{
	struct ScriptInterface *si;
	Rt_ArrayForEach(si, &f_scriptIfaces)
		Sys_Free(si->script);

	Rt_TermArray(&f_scriptIfaces);
}

static void *
LuaAlloc(void *ud, void *ptr, size_t oSize, size_t nSize)
{
	void *new = NULL;

	if (nSize == 0)
		Sys_Free(ptr);
	else
		new = Sys_AlignedReAlloc(ptr, nSize, 1, 16, MH_Script);

	return new;
}

static int
LuaSearcher(lua_State *vm)
{
	const char *name = lua_tostring(vm, 1);
	const uint64_t hash = Rt_HashString(name);

	struct ScriptInterface *si;
	Rt_ArrayForEach(si, &f_scriptIfaces) {
		if (si->hash != hash)
			continue;

		if (si->open) {
			lua_pushcfunction(vm, si->open);
		} else {
			if (luaL_loadbufferx(vm, si->script, si->scriptLen, name, "t") != LUA_OK)
				return lua_error(vm);

			lua_pcall(vm, 0, LUA_MULTRET, 0);
			if (!lua_istable(vm, -1)) {
				return lua_error(vm);
			}

			lua_pushvalue(vm, -1);
			lua_setfield(vm, LUA_REGISTRYINDEX, name);

			if (luaL_loadbufferx(vm, si->script, si->scriptLen, name, "t") != LUA_OK)
				return lua_error(vm);
		}

		return 1;
	}

	lua_pushfstring(vm, "Module for interface %s not found", name);
	return 1;
}

/* NekoEngine
 *
 * Script.c
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
