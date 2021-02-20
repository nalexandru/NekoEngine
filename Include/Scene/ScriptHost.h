#ifndef _SCN_SCRIPT_HOST_H_
#define _SCN_SCRIPT_HOST_H_

#include <lua.h>

#include <Engine/Component.h>

struct ScriptHost
{
	COMPONENT_BASE;

	lua_State *vm;
};

bool Scn_InitScriptHost(struct ScriptHost *s, const void **args);
void Scn_TermScriptHost(struct ScriptHost *s);

void Scn_ExecScriptHosts(void **comp, void *args);

#endif /* _SCN_SCRIPT_HOST_H_ */
