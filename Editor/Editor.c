#include <stdio.h>
#include <assert.h>

#include <Scene/Scene.h>
#include <System/System.h>
#include <Engine/Config.h>
#include <Engine/Events.h>
#include <Engine/Version.h>
#include <Engine/Application.h>

#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Editor/Project.h>
#include <Editor/Asset/Import.h>

#ifndef _countof
#	define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

struct NeApplicationInfo App_applicationInfo =
{
	"NekoEditor",
	E_CPY_STR,
	{ E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD, E_VER_REVISION }
};

char Ed_dataDir[ED_MAX_PATH] = { 0 };

static uint64_t _sceneLoadedEvt;

static void App_SceneLoaded(void *user, void *args);

bool
App_InitApplication(int argc, char *argv[])
{
	Ed_ShowProjectDialog();

	if (!Ed_activeProject)
		return false;

	const char *dataDir = E_GetCVarStr("Engine_DataDir", "Data")->str;

#ifdef SYS_PLATFORM_WINDOWS
	if (Ed_dataDir[1] != ':') {
#else
	if (Ed_dataDir[0] != '/') {
#endif
		char *buff = Sys_Alloc(sizeof(*buff), 4096, MH_Transient);
		getcwd(buff, 4096);

		snprintf(Ed_dataDir, sizeof(Ed_dataDir), "%s%c%s", buff, ED_DIR_SEPARATOR, dataDir);
	} else {
		snprintf(Ed_dataDir, sizeof(Ed_dataDir), "%s", dataDir);
	}

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
	Scn_ActivateScene((struct NeScene *)args);
}
