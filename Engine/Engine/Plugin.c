#include <System/Log.h>
#include <Engine/Plugin.h>
#include <Runtime/Runtime.h>

#define PLUGIN_MOD	"Plugin"

struct NePluginInfo
{
	struct NePlugin *plugin;
	bool loaded;
	NePluginInitProc init;
	NePluginTermProc term;
};

static struct NeArray _pluginList;

bool
E_InitPluginList(void)
{
	if (!Rt_InitArray(&_pluginList, 10, sizeof(struct NePluginInfo), MH_System))
		return false;

	char buff[4096];
	Sys_ExecutableLocation(buff, sizeof(buff));

	Sys_LogEntry(PLUGIN_MOD, LOG_DEBUG, "Exe path: %s", buff);

	// get exe path
	// search for shared libraries in $(exepath)/Plugins
	// attempt to load plugin info from each one

	return true;
}

void
E_LoadPlugins(enum NePluginLoadOrder order)
{
	struct NePluginInfo *p;
	Rt_ArrayForEach(p, &_pluginList) {
		if (p->plugin->loadOrder != order)
			continue;

		if (!p->init()) {
			Sys_LogEntry(PLUGIN_MOD, LOG_CRITICAL, "Plugin %s failed initialization", p->plugin->name);
			continue;
		}

		p->loaded = true;
	}
}

void
E_UnloadPlugins(enum NePluginLoadOrder order)
{
	struct NePluginInfo *p;
	Rt_ArrayForEach(p, &_pluginList) {
		if (p->plugin->loadOrder != order)
			continue;

		p->term();
		p->loaded = false;
	}
}

void
E_TermPluginList(void)
{
	Rt_TermArray(&_pluginList);
}
