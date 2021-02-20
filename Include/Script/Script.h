#ifndef _E_SCRIPT_H_
#define _E_SCRIPT_H_

#include <stdbool.h>

#include <lua.h>

lua_State *E_CreateVM(bool jit);

bool E_LoadScript(lua_State *vm, const char *source);
bool E_LoadScriptFile(lua_State *vm, const char *path);

void E_DestroyVM(lua_State *vm);

bool E_InitScriptSystem(void);
void E_TermScriptSystem(void);

#endif /* _E_SCRIPT_H_ */
