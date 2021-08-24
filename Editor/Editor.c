#include <assert.h>

#include <Scene/Scene.h>
#include <Engine/Config.h>
#include <Engine/Events.h>
#include <Engine/Version.h>
#include <Engine/Application.h>

#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Editor/Asset/Import.h>

#ifndef _countof
#	define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

struct ApplicationInfo App_applicationInfo =
{
	L"NekoEditor",
	E_CPY_STR,
	{ E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD, E_VER_REVISION }
};

const char *Ed_dataDir = NULL;

static uint64_t _sceneLoadedEvt;

static void App_SceneLoaded(void *user, void *args);

bool
App_InitApplication(int argc, char *argv[])
{
	Ed_dataDir = E_GetCVarStr(L"Engine_DataDir", "Data")->str;

	if (!Init_AssetImporter())
		return false;

	_sceneLoadedEvt = E_RegisterHandler(EVT_SCENE_LOADED, App_SceneLoaded, NULL);
	
	Scn_StartSceneLoad("/Scenes/EditorTest.scn");

	return Ed_CreateGUI();;
}

void
App_Frame(void)
{
#ifndef __APPLE__
	EdGUI_ProcessEvents();
#endif
}

void
App_TermApplication(void)
{
	Ed_TermGUI();

	Term_AssetImporter();
}

void
App_SceneLoaded(void *user, void *args)
{
	Scn_ActivateScene((struct Scene *)args);
}
