#include <stdio.h>
#include <stdbool.h>

#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Audio/Audio.h>
#include <Input/Input.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <System/Log.h>
#include <System/Endian.h>
#include <System/Memory.h>
#include <System/System.h>
#include <System/Window.h>
#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Render/Render.h>
#include <Render/Model.h>
#include <Render/Material.h>
#include <Engine/Event.h>
#include <Engine/Entity.h>
#include <Engine/Version.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Engine/Component.h>
#include <Engine/Application.h>
#include <Script/Script.h>
#include <UI/UI.h>

#include <Render/Texture.h>
#include <Render/ModelRender.h>

#include "ECS.h"

#define EMOD	L"Engine"

#define E_CONFIG_FILE	"Data/Config/Engine.ini"

void *E_Screen = NULL;
uint32_t *E_ScreenWidth = NULL;
uint32_t *E_ScreenHeight = NULL;
double E_DeltaTime = 0.0;

static bool _shutdown;
static double _startTime, _prevTime;

bool
E_Init(int argc, char *argv[])
{
	int opt;
	wchar_t titleBuff[256];
	const char *configFile = E_CONFIG_FILE;
	const char *logFile = NULL;
	const char *dataDir = NULL;

	while ((opt = getopt(argc, argv, "c:d:l:n")) != -1) {
		switch (opt) {
		case 'c':
			configFile = optarg;
			break;
		case 'd':
			dataDir = optarg;
			break;
		case 'l':
			logFile = optarg;
			break;
		}
	}
	
	E_InitConfig(configFile);

	if (logFile)
		E_SetCVarStr(L"Engine_LogFile", logFile);

	if (dataDir)
		E_SetCVarStr(L"Engine_DataDir", dataDir);

	E_ScreenWidth = &E_GetCVarU32(L"Engine_ScreenWidth", 1280)->u32;
	E_ScreenHeight = &E_GetCVarU32(L"Engine_ScreenHeight", 720)->u32;

	Sys_Init();

	Sys_InitMemory();

	if (App_ApplicationInfo.version.revision)
		Sys_LogEntry(EMOD, LOG_INFORMATION, L"%ls v%d.%d.%d.%d", App_ApplicationInfo.name, App_ApplicationInfo.version.major,
			App_ApplicationInfo.version.minor, App_ApplicationInfo.version.build, App_ApplicationInfo.version.revision);
	else
		Sys_LogEntry(EMOD, LOG_INFORMATION, L"%ls v%d.%d.%d", App_ApplicationInfo.name, App_ApplicationInfo.version.major,
			App_ApplicationInfo.version.minor, App_ApplicationInfo.version.build);
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Copyright \u00A9 %ls", App_ApplicationInfo.copyright);

	Sys_LogEntry(EMOD, LOG_INFORMATION, L"%ls \"%ls\" v%ls", E_PGM_NAME, E_CODENAME, E_VER_STR);
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Copyright \u00A9 %ls", E_CPY_STR);
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Starting up...");

	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Host: %s", Sys_Hostname());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"CPU: %s", Sys_CpuName());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"\tCount: %d", Sys_NumCpus());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"\tArchitecture: %s", Sys_Machine());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"\tBig Endian: %s", Sys_BigEndian() ? "True" : "False");

	Sys_CreateWindow();

	E_InitJobSystem();
	E_InitIOSystem(argv ? argv[0] : "NekoEngine");
	E_InitEventSystem();

	E_InitComponents();
	E_InitEntities();
	E_InitECSystems();

	E_InitResourceSystem();

	Re_Init();
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"GPU: %ls", Re_RenderInfo.device);
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"\tRendering API: %ls", Re_RenderInfo.name);

	E_RegisterResourceType(RES_MODEL, RE_APPEND_DATA_SIZE(struct Model, Re_ModelRenderDataSize),
		(ResourceCreateProc)Re_CreateModel, (ResourceLoadProc)Re_LoadModel, (ResourceUnloadProc)Re_UnloadModel);
	E_RegisterResourceType(RES_TEXTURE, RE_APPEND_DATA_SIZE(struct Texture, Re_TextureRenderDataSize),
		(ResourceCreateProc)Re_CreateTexture, (ResourceLoadProc)Re_LoadTexture, (ResourceUnloadProc)Re_UnloadTexture);

	Re_LoadMaterials();

	Au_Init();

	In_InitInput();
	UI_InitUI();

	if (App_ApplicationInfo.version.revision)
		swprintf(titleBuff, 256, L"%ls v%u.%u.%u.%u - %ls - %ls", App_ApplicationInfo.name,
			App_ApplicationInfo.version.major, App_ApplicationInfo.version.minor, App_ApplicationInfo.version.build,
			App_ApplicationInfo.version.revision, Re_RenderInfo.device, Re_RenderInfo.name);
	else
		swprintf(titleBuff, 256, L"%ls v%u.%u.%u - %ls - %ls", App_ApplicationInfo.name,
			App_ApplicationInfo.version.major, App_ApplicationInfo.version.minor, App_ApplicationInfo.version.build,
			Re_RenderInfo.device, Re_RenderInfo.name);

	Sys_SetWindowTitle(titleBuff);

	E_InitScriptSystem();

	_startTime = (double)Sys_Time();

	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Engine start up complete.");

	if (!App_InitApplication(argc, argv))
		return false;
	
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Application started.");

	return true;
}

void
E_Term(void)
{
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Shutting down...");

	App_TermApplication();

	if (Scn_ActiveScene)
		Scn_UnloadScene(Scn_ActiveScene);

	E_TermScriptSystem();

	UI_TermUI();
	In_TermInput();

	E_TermECSystems();
	E_TermEntities();
	E_TermComponents();

	Re_UnloadMaterials();

	E_PurgeResources();

	Au_Term();
	Re_Term();

	E_TermResourceSystem();

	E_TermEventSystem();
	E_TermIOSystem();
	E_TermJobSystem();

	Sys_DestroyWindow();

	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Shut down complete.");

	Sys_TermMemory();
	Sys_Term();

	E_TermConfig();
}

int
E_Run(void)
{
	while (!_shutdown) {
		if (!Sys_ProcessEvents()) {
			_shutdown = true;
			break;
		}

		if (Sys_ScreenVisible())
			E_Frame();
	}

	E_Term();

	return 0;
}

void
E_Frame(void)
{
	double now = E_Time();
	E_DeltaTime = now - _prevTime;
	_prevTime = now;

	if (!Scn_ActiveScene || !Scn_ActiveCamera) {
		E_ProcessEvents();
		return;
	}

	E_ExecuteSystemGroupS(Scn_ActiveScene, ECSYS_GROUP_LOGIC);

	E_ProcessEvents();

	E_ExecuteSystemGroupS(Scn_ActiveScene, ECSYS_GROUP_POST_LOGIC);

	E_ExecuteSystemGroupS(Scn_ActiveScene, ECSYS_GROUP_PRE_RENDER);
	Re_RenderFrame();
	E_ExecuteSystemGroupS(Scn_ActiveScene, ECSYS_GROUP_POST_RENDER);

	In_Update();

	Sys_ResetHeap(MH_Transient);
}

double
E_Time(void)
{
	return ((double)Sys_Time() - _startTime) * 0.000000001;
}

void
E_Shutdown(void)
{
	_shutdown = true;
}

