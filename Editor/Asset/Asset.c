#include <System/Log.h>
#include <Engine/Job.h>
#include <Engine/Engine.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Scene/Scene.h>
#include <Scene/Components.h>
#include <Runtime/Runtime.h>

#include <Editor/GUI.h>
#include <Editor/Asset/Asset.h>

#define ED_A_MOD	"Asset"

struct OpenJobArgs
{
	struct NeAssetOpenHandler *handler;
	char *path;
};

static bool _MatchMesh(const char *path) { return strstr(path, ".nmesh") != NULL; }
static bool _OpenMesh(const char *path);
static void _OpenJob(int worker, struct OpenJobArgs *args);

struct NeAssetOpenHandler
{
	bool (*Match)(const char *path);
	bool (*Open)(const char *path);
};

static struct NeAssetOpenHandler _handlers[] =
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

		struct OpenJobArgs *args = Sys_Alloc(sizeof(*args), 1, MH_Editor);
		args->handler = &_handlers[i];
		args->path = Rt_StrDup(path, MH_Editor);

	#ifndef SYS_PLATFORM_WINDOWS
		E_ExecuteJob((NeJobProc)_OpenJob, args, NULL, NULL);
	#else
		_OpenJob(0, args);
	#endif

		break;
	}
}

static bool
_OpenMesh(const char *path)
{
	char buff[2048];
	snprintf(buff, 2048, "Loading %s...", path);
	EdGUI_ShowProgressDialog(buff);

	NeEntityHandle eh = E_CreateEntity(NULL);

	E_AddNewComponent(eh, E_ComponentTypeId(TRANSFORM_COMP), NULL);

	const void *args[] = { "Model", path, NULL };
	E_AddNewComponent(eh, E_ComponentTypeId(MODEL_RENDER_COMP), args);

	char *str = Rt_TransientStrDup(path);
	char *name = strrchr(str, '/');
	*name++ = 0x0;

	char *ptr = strrchr(name, '.');
	*ptr = 0x0;

	E_RenameEntity(eh, name);

	EdGUI_HideProgressDialog();

	return true;
}

static void
_OpenJob(int worker, struct OpenJobArgs *args)
{
	if (!args->handler->Open(args->path)) {
		EdGUI_MessageBox("Error", "Failed to open asset");
		Sys_LogEntry(ED_A_MOD, LOG_CRITICAL, "Failed to open asset from file: %s", args->path);
	}
	Sys_Free(args->path);
	Sys_Free(args);
}
