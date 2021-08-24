#include <System/Log.h>
#include <Engine/Engine.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Scene/Scene.h>
#include <Scene/Components.h>
#include <Runtime/Runtime.h>

#include <Editor/GUI.h>
#include <Editor/Asset/Asset.h>

#define ED_A_MOD	L"Asset"

static bool _MatchMesh(const char *path) { return strstr(path, ".nmesh") != NULL; }
static bool _OpenMesh(const char *path);

struct AssetOpenHandler
{
	bool (*Match)(const char *path);
	bool (*Open)(const char *path);
};

static struct AssetOpenHandler _handlers[] =
{
	{
		.Match = _MatchMesh,
		.Open = _OpenMesh
	}
};

void
Ed_OpenAsset(const char *path)
{
	for (size_t i = 0; i < sizeof(_handlers) / sizeof(_handlers[0]); ++i) {
		if (!_handlers[i].Match(path))
			continue;

		if (!_handlers->Open(path)) {
			EdGUI_MessageBox("Error", "Failed to open asset");
			Sys_LogEntry(ED_A_MOD, LOG_CRITICAL, L"Failed to open asset from file: %hs", path);
		}

		break;
	}
}

static bool
_OpenMesh(const char *path)
{
	EntityHandle eh = E_CreateEntity(NULL);

	E_AddNewComponent(eh, E_ComponentTypeId(TRANSFORM_COMP), NULL);

	const void *args[] = { "Model", path, NULL };
	E_AddNewComponent(eh, E_ComponentTypeId(MODEL_RENDER_COMP), args);

	char *str = Rt_TransientStrDup(path);
	char *name = strrchr(str, '/');
	*name++ = 0x0;

	char *ptr = strrchr(name, '.');
	*ptr = 0x0;

	E_RenameEntity(eh, Rt_MbsToWcs(name));

	return true;
}
