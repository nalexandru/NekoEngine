#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <Engine/IO.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Script/Script.h>

#include "Interface.h"

#define SCRIPTMOD	L"Script"

lua_State *
E_CreateVM(bool jit)
{
	lua_State *vm;

	vm = luaL_newstate();
	if (!vm)
		return NULL;

	luaL_openlibs(vm);
	
	SIface_OpenRender(vm);

	return vm;
}

bool
E_LoadScript(lua_State *vm, const char *source)
{
	if (luaL_dostring(vm, source) && lua_gettop(vm)) {
		Sys_LogEntry(SCRIPTMOD, LOG_CRITICAL, L"Failed to load script: %s", lua_tostring(vm, -1));
		lua_pop(vm, 1);
		return false;
	}

	return true;
}

bool
E_LoadScriptFile(lua_State *vm, const char *path)
{
	int64_t len;
	char *source;
	struct Stream stm;

	if (!E_FileStream(path, IO_READ, &stm))
		return false;

	len = E_StreamLength(&stm);
	source = Sys_Alloc((size_t)len, 1, MH_Transient);

	E_ReadStream(&stm, source, E_StreamLength(&stm));
	E_CloseStream(&stm);

	if (luaL_dostring(vm, source) && lua_gettop(vm)) {
		Sys_LogEntry(SCRIPTMOD, LOG_CRITICAL, L"Failed to load script: %s", lua_tostring(vm, -1));
		lua_pop(vm, 1);
		return false;
	}

	return true;
}

void
E_DestroyVM(lua_State *vm)
{
	lua_close(vm);
}

bool
E_InitScriptSystem(void)
{
	return true;
}

void
E_TermScriptSystem(void)
{
	//
}

