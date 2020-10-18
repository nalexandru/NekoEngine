#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <System/Log.h>
#include <Script/Script.h>
#include <Scene/ScriptHost.h>

bool
Scn_InitScriptHost(struct ScriptHost *s, const void **args)
{
	bool rc = true;
	const char *file = NULL, *src = NULL;

	for (args; *args; ++args) {
		size_t len;
		const char *arg;
		
		len = strlen(*args);
		arg = *(++args);

		if (!strncmp(*args, "file", len))
			file = arg;
		else if (!strncmp(*args, "source", len))
			src = arg;
		else if (!strncmp(*args, "jit", len))
			rc = !strncmp(arg, "true", 4);
	}
	
	if (!file && !src)
		return false;

	s->vm = E_CreateVM(rc);
	if (!s->vm)
		return false;

	if (file)
		rc = E_LoadScriptFile(s->vm, file);
	else
		rc = E_LoadScript(s->vm, src);

	if (!rc)
		E_DestroyVM(s->vm);

	return rc;
}

void
Scn_TermScriptHost(struct ScriptHost *s)
{
	E_DestroyVM(s->vm);
}

void
Scn_ExecScriptHosts(void **comp, void *args)
{
//	struct ScriptHost *sh = comp[0];

	// ...
}

