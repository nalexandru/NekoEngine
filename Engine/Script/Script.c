#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <Engine/IO.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Script/Script.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

#define SCRIPTMOD	"Script"

static const luaL_Reg _luaLibs[] = {
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

static struct NeArray _initScripts;
static void *_luaAlloc(void *ud, void *ptr, size_t oSize, size_t nSize);

lua_State *
Sc_CreateVM(void)
{
	lua_State *vm;

	vm = luaL_newstate_alloc(_luaAlloc);
	if (!vm)
		return NULL;

	lua_setallocf(vm, _luaAlloc, NULL);

	for (const luaL_Reg *lib = _luaLibs; lib->func; ++lib) {
		luaL_requiref(vm, lib->name, lib->func, 1);
		lua_pop(vm, 1);
	}
	
	// TODO: make engine libraries follow Lua library conventions
	SIface_OpenSystem(vm);
	SIface_OpenEngine(vm);
	SIface_OpenRender(vm);
	SIface_OpenConsole(vm);
	SIface_OpenUI(vm);
	SIface_OpenScriptComponent(vm);
	SIface_OpenResource(vm);
	SIface_OpenIO(vm);
	SIface_OpenEvent(vm);
	SIface_OpenInput(vm);
	SIface_OpenTransform(vm);
	SIface_OpenCamera(vm);
	SIface_OpenLight(vm);
	SIface_OpenConfig(vm);
	
	const char *initScript = NULL;
	Rt_ArrayForEachPtr(initScript, &_initScripts) {
		if (luaL_dostring(vm, initScript) && lua_gettop(vm)) {
			Sys_LogEntry(SCRIPTMOD, LOG_CRITICAL, "Failed to execute component init script: %s", lua_tostring(vm, -1));
			lua_pop(vm, 1);
		}
	}

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
		return false;

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
	int64_t len;
	char *source;
	struct NeStream stm;

	if (!E_FileStream(path, IO_READ, &stm))
		return false;

	len = E_StreamLength(&stm);
	source = Sys_Alloc((size_t)len, 1, MH_Transient);

	E_ReadStream(&stm, source, E_StreamLength(&stm));
	E_CloseStream(&stm);

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
Sc_RegisterInitScript(const char *script)
{
	return Rt_ArrayAddPtr(&_initScripts, Rt_StrDup(script, MH_Script));
}

void
Sc_DestroyVM(lua_State *vm)
{
	lua_close(vm);
}

bool
Sc_InitScriptSystem(void)
{
	Rt_InitPtrArray(&_initScripts, 10, MH_Script);

	return true;
}

void
Sc_TermScriptSystem(void)
{
	char *ptr;
	Rt_ArrayForEachPtr(ptr, &_initScripts)
		Sys_Free(ptr);

	Rt_TermArray(&_initScripts);
}

static void *
_luaAlloc(void *ud, void *ptr, size_t oSize, size_t nSize)
{
	void *new = NULL;

	if (nSize == 0)
		Sys_Free(ptr);
	else
		new = Sys_ReAlloc(ptr, nSize, 1, MH_Script);

	return new;
}
