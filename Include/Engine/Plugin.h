#ifndef _NE_ENGINE_PLUGIN_H_
#define _NE_ENGINE_PLUGIN_H_

#include <Engine/Types.h>

#define NE_PLUGIN_ID	0xB16B00B5
#define NE_PLUGIN_API	1

enum NePluginLoadOrder
{
	NEP_LOAD_EARLY_INIT,
	NEP_LOAD_PRE_IO,
	NEP_LOAD_PRE_SCRIPTING,
	NEP_LOAD_PRE_ECS,
	NEP_LOAD_PRE_RESOURCES,
	NEP_LOAD_PRE_RENDER,
	NEP_LOAD_PRE_AUDIO,
	NEP_LOAD_PRE_INPUT,
	NEP_LOAD_POST_INIT,
	NEP_LOAD_POST_APP_INIT,

	NEP_FORCE_UINT32 = (uint32_t)-1
};

struct NePlugin
{
	uint32_t identifier;
	uint32_t apiVersion;

	const char name[64];
	const char copyright[64];
	struct NeVersion version;
	enum NePluginLoadOrder loadOrder;
};

typedef bool (*NePluginInitProc)(void);
typedef void (*NePluginTermProc)(void);

#ifdef _ENGINE_INTERNAL_

bool E_InitPluginList(void);
void E_LoadPlugins(enum NePluginLoadOrder order);
void E_UnloadPlugins(enum NePluginLoadOrder order);
void E_TermPluginList(void);

#endif

#endif /* _NE_ENGINE_PLUGIN_H_ */
