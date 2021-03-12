#include <stdio.h>
#include <stdbool.h>

#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Audio/Audio.h>
#include <Input/Input.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <System/Log.h>
#include <System/Thread.h>
#include <System/Endian.h>
#include <System/Memory.h>
#include <System/System.h>
#include <System/Window.h>
#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Render/Driver.h>
//#include <Render/Material.h>
#include <Engine/Event.h>
#include <Engine/Entity.h>
#include <Engine/Version.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Engine/Component.h>
#include <Engine/Application.h>
#include <Script/Script.h>
#include <UI/UI.h>

//#include <Render/Texture.h>
//#include <Render/Components/ModelRender.h>

#include "ECS.h"

#define EMOD	L"Engine"

#define E_CONFIG_FILE	"Data/Config/Engine.ini"

void *E_screen = NULL;
uint32_t *E_screenWidth = NULL;
uint32_t *E_screenHeight = NULL;
double E_deltaTime = 0.0;

static bool _shutdown;
static double _startTime, _prevTime;
#include <Math/sanity.h>

bool
E_Init(int argc, char *argv[])
{
	int opt;
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

	Sys_Init();
	Sys_InitMemory();

	__MathDbg_SanityTest();

	if (logFile)
		E_SetCVarStr(L"Engine_LogFile", logFile);
	
	if (dataDir)
		E_SetCVarStr(L"Engine_DataDir", dataDir);
	
	E_screenWidth = &E_GetCVarU32(L"Engine_ScreenWidth", 1280)->u32;
	E_screenHeight = &E_GetCVarU32(L"Engine_ScreenHeight", 720)->u32;
	
	if (App_applicationInfo.version.revision)
		Sys_LogEntry(EMOD, LOG_INFORMATION, L"%ls v%d.%d.%d.%d", App_applicationInfo.name, App_applicationInfo.version.major,
			App_applicationInfo.version.minor, App_applicationInfo.version.build, App_applicationInfo.version.revision);
	else
		Sys_LogEntry(EMOD, LOG_INFORMATION, L"%ls v%d.%d.%d", App_applicationInfo.name, App_applicationInfo.version.major,
			App_applicationInfo.version.minor, App_applicationInfo.version.build);
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Copyright (C) %ls", App_applicationInfo.copyright);

	Sys_LogEntry(EMOD, LOG_INFORMATION, L"%ls \"%ls\" v%ls", E_PGM_NAME, E_CODENAME, E_VER_STR);
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Copyright (C) %ls", E_CPY_STR);
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Starting up...");

	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Host: %hs", Sys_Hostname());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Platform: %hs %hs", Sys_OperatingSystem(), Sys_OperatingSystemVersion());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"CPU: %hs", Sys_CpuName());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"\tFrequency: %d MHz", Sys_CpuFreq());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"\tCount: %d", Sys_NumCpus());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"\tArchitecture: %hs", Sys_Machine());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"\tBig Endian: %hs", Sys_BigEndian() ? "yes" : "no");

	Sys_CreateWindow();

	E_InitJobSystem();
	E_InitIOSystem(argv ? argv[0] : "NekoEngine");
	E_InitEventSystem();

	E_InitComponents();
	E_InitEntities();
	E_InitECSystems();

	E_InitResourceSystem();

	Re_InitRender();

//	E_RegisterResourceType(RES_MODEL, RE_APPEND_DATA_SIZE(struct Model, Re.modelRenderDataSize),
//		(ResourceCreateProc)Re_CreateModel, (ResourceLoadProc)Re_LoadModel, (ResourceUnloadProc)Re_UnloadModel);
//	E_RegisterResourceType(RES_TEXTURE, RE_APPEND_DATA_SIZE(struct Texture, Re.textureRenderDataSize),
//		(ResourceCreateProc)Re_CreateTexture, (ResourceLoadProc)Re_LoadTexture, (ResourceUnloadProc)Re_UnloadTexture);

//	Re_LoadMaterials();

	Au_Init();

	In_InitInput();
//	UI_InitUI();
	
#ifdef _DEBUG
	wchar_t titleBuff[256];
	
	swprintf(titleBuff, sizeof(titleBuff) / sizeof(wchar_t), L"%ls v%u.%u.%u", App_applicationInfo.name,
		App_applicationInfo.version.major, App_applicationInfo.version.minor, App_applicationInfo.version.build);

	if (App_applicationInfo.version.revision)
		swprintf(titleBuff + wcslen(titleBuff), sizeof(titleBuff) / sizeof(wchar_t) - wcslen(titleBuff),
			L".%u", App_applicationInfo.version.revision);
	
	swprintf(titleBuff + wcslen(titleBuff), sizeof(titleBuff) / sizeof(wchar_t) - wcslen(titleBuff),
		L" - NekoEngine v%u.%u.%u", E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD);
	
	if (E_VER_REVISION)
		swprintf(titleBuff + wcslen(titleBuff), sizeof(titleBuff) / sizeof(wchar_t) - wcslen(titleBuff),
			L".%u", E_VER_REVISION);
	
	swprintf(titleBuff + wcslen(titleBuff), sizeof(titleBuff) / sizeof(wchar_t) - wcslen(titleBuff),
		L" - GPU: %hs (%ls)", Re_deviceInfo.deviceName, Re_driver->driverName);
	
	Sys_SetWindowTitle(titleBuff);
#else
	Sys_SetWindowTitle(App_applicationInfo.name);
#endif
	
	E_InitScriptSystem();

	_startTime = (double)Sys_Time();

	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Engine start up complete.");

	if (!App_InitApplication(argc, argv))
		return false;
	
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Application started.");
	
	Sys_ResetHeap(MH_Transient);

	return true;
}

void
E_Term(void)
{
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Shutting down...");

	App_TermApplication();

	if (Scn_activeScene)
		Scn_UnloadScene(Scn_activeScene);

	E_TermScriptSystem();

//	UI_TermUI();
	In_TermInput();

	E_TermECSystems();
	E_TermEntities();
	E_TermComponents();

//	Re_UnloadMaterials();

	E_PurgeResources();

	Au_Term();
	Re_TermRender();

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
		
		Sys_ResetHeap(MH_Transient);
	}

	E_Term();

	return 0;
}

void
E_Frame(void)
{
	double now = E_Time();
	E_deltaTime = now - _prevTime;
	_prevTime = now;

	if (!Scn_activeScene || !Scn_activeCamera) {
		E_ProcessEvents();
		Re_RenderFrame();
		return;
	}

	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_LOGIC);

	E_ProcessEvents();

	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_POST_LOGIC);

	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_PRE_RENDER);
	Re_RenderFrame();
	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_POST_RENDER);

	In_Update();
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
