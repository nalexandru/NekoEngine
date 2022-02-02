#ifndef _NE_SCRIPT_SCRIPT_H_
#define _NE_SCRIPT_SCRIPT_H_

#include <stdbool.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

lua_State *Sc_CreateVM(void);

const char *Sc_Execute(lua_State *vm, const char *command);
const char *Sc_ExecuteFile(lua_State *vm, const char *path);

bool Sc_LoadScript(lua_State *vm, const char *source);
bool Sc_LoadScriptFile(lua_State *vm, const char *path);

void Sc_LogStackDump(lua_State *vm, int severity);

bool Sc_RegisterInitScript(const char *script);

void Sc_DestroyVM(lua_State *vm);

bool Sc_InitScriptSystem(void);
void Sc_TermScriptSystem(void);

#endif /* _NE_SCRIPT_SCRIPT_H_ */
