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
#include <Render/Driver/Driver.h>
#include <Engine/Event.h>
#include <Engine/Events.h>
#include <Engine/Entity.h>
#include <Engine/Version.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Engine/Component.h>
#include <Engine/Application.h>
#include <Script/Script.h>
#include <UI/UI.h>

#include "ECS.h"

#define EMOD	L"Engine"

#define E_CONFIG_FILE	"Data/Config/Engine.ini"

void *E_screen = NULL;
uint32_t *E_screenWidth = NULL;
uint32_t *E_screenHeight = NULL;
double E_deltaTime = 0.0;

static bool _shutdown;
static double _startTime, _prevTime;
static int32_t *_frameLimiter = NULL;

#include <Math/sanity.h>

struct EngineSubsystem
{
	char name[256];
	bool (*init)(void);
	void (*term)(void);
};

static struct EngineSubsystem _subsystems[] =
{
	{ "Window", Sys_CreateWindow, Sys_DestroyWindow },
	{ "Job System", E_InitJobSystem, E_TermJobSystem },
	{ "I/O System", E_InitIOSystem, E_TermIOSystem },
	{ "Event System", E_InitEventSystem, E_TermEventSystem },
	{ "Components", E_InitComponents, E_TermComponents },
	{ "Entities", E_InitEntities, E_TermEntities },
	{ "ECSystems", E_InitECSystems, E_TermECSystems },
	{ "Resource System", E_InitResourceSystem, E_TermResourceSystem },
	{ "Render System", Re_InitRender, Re_TermRender },
	{ "Audio System", Au_Init, Au_Term },
	{ "Resource Purge", NULL, E_PurgeResources },
	{ "Input", In_InitInput, In_TermInput },
	{ "UI", UI_InitUI, UI_TermUI },
	{ "Scripting", Sc_InitScriptSystem, Sc_TermScriptSystem }
};
static const int32_t _subsystemCount = sizeof(_subsystems) / sizeof(_subsystems[0]);

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

	if (logFile)
		E_SetCVarStr(L"Engine_LogFile", logFile);

	if (dataDir)
		E_SetCVarStr(L"Engine_DataDir", dataDir);

	//__MathDbg_SanityTest();

	E_screenWidth = &E_GetCVarU32(L"Engine_ScreenWidth", 1280)->u32;
	E_screenHeight = &E_GetCVarU32(L"Engine_ScreenHeight", 853)->u32;

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
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"\tCount: %d / %d", Sys_CpuCount(), Sys_CpuThreadCount());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"\tArchitecture: %hs", Sys_Machine());
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"\tBig Endian: %hs", Sys_BigEndian() ? "yes" : "no");

	for (int32_t i = 0; i < _subsystemCount; ++i) {
		if (!_subsystems[i].init)
			continue;

		if (!_subsystems[i].init()) {
			wchar_t *msg = Sys_Alloc(sizeof(*msg), 256, MH_Transient);
			swprintf(msg, 256, L"Failed to initialize %hs. The program will now exit.", _subsystems[i].name);
			Sys_MessageBox(L"Fatal Error", msg, MSG_ICON_ERROR);

			Sys_LogEntry(EMOD, LOG_CRITICAL, L"Failed to initialize %hs", _subsystems[i].name);
			return false;
		}
	}

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

	_startTime = (double)Sys_Time();
	_frameLimiter = &E_GetCVarI32(L"Engine_FrameLimiter", 0)->i32;

	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Engine start up complete.");

	if (!App_InitApplication(argc, argv))
		return false;

	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Application started.");

	Sys_ResetHeap(MH_Transient);

	lua_State *vm = Sc_CreateVM(true);
	Sc_LoadScriptFile(vm, "/Scripts/test.lua");
	Sc_DestroyVM(vm);

	return true;
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
	static double nextFrame = 0;

	if (nextFrame > now) {
		Sys_Yield();
		return;
	}

	E_deltaTime = now - _prevTime;
	_prevTime = now;

	if (!Scn_activeScene || !Scn_activeCamera) {
		E_ProcessEvents();
		App_Frame();

		return;
	}

	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_LOGIC);
	E_ProcessEvents();
	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_POST_LOGIC);

	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_PRE_RENDER);
	Re_RenderFrame();
	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_POST_RENDER);

	In_Update();

	App_Frame();

	if (*_frameLimiter)
		nextFrame = now + (1.0 / *_frameLimiter);
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

void
E_ScreenResized(uint32_t width, uint32_t height)
{
	*E_screenWidth = width;
	*E_screenHeight = height;

	if (Re_device)
		Re_deviceProcs.ScreenResized(Re_device, Re_swapchain);

	E_Broadcast(EVT_SCREEN_RESIZED, NULL);
}

void
E_Term(void)
{
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Shutting down...");

	Re_WaitIdle();

	App_TermApplication();

	if (Scn_activeScene)
		Scn_UnloadScene(Scn_activeScene);

	for (int32_t i = _subsystemCount - 1; i >= 0; --i) {
		if (!_subsystems[i].term)
			continue;
		_subsystems[i].term();
	}

	Sys_LogMemoryStatistics();
	Sys_LogEntry(EMOD, LOG_INFORMATION, L"Shut down complete.");

	E_TermConfig();
	Sys_TermMemory();
	Sys_Term();
}
