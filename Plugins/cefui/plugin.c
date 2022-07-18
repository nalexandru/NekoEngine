#include <Engine/Plugin.h>

#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif

EXPORT struct NePlugin NePluginInfo =
{
	.identifier = NE_PLUGIN_ID,
	.apiVersion = NE_PLUGIN_API,
	.name = "",
	.copyright = "",
	.version = { 0, 1, 0, 1 },
	.loadOrder = NEP_LOAD_POST_INIT
};

EXPORT bool
InitPlugin(void)
{
	return true;
}

EXPORT void
TermPlugin(void)
{
	//
}
