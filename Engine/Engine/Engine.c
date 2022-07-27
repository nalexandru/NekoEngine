#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Audio/Audio.h>
#include <Input/Input.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Engine/Plugin.h>
#include <System/Log.h>
#include <System/Thread.h>
#include <System/Endian.h>
#include <System/Memory.h>
#include <System/System.h>
#include <System/Window.h>
#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Render/Render.h>
#include <Render/Backend.h>
#include <Engine/XR.h>
#include <Engine/Event.h>
#include <Engine/Events.h>
#include <Engine/Console.h>
#include <Engine/Version.h>
#include <Engine/Profiler.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Engine/Application.h>
#include <Network/Network.h>
#include <Script/Script.h>
#include <UI/UI.h>

#include "ECS.h"

#define EMOD	"Engine"

#define E_CONFIG_FILE	"Data/Config/Engine.ini"

void *E_screen = NULL;
uint32_t *E_screenWidth = NULL;
uint32_t *E_screenHeight = NULL;
double E_deltaTime = 0.0;

static bool _shutdown;
static double _startTime, _prevTime;
static int32_t *_frameLimiter = NULL;

struct NeEngineSubsystem
{
	char name[256];
	bool (*init)(void);
	void (*term)(void);
	enum NePluginLoadOrder loadPlugins;
};

static struct NeEngineSubsystem _subsystems[] =
{
	{ "Window", Sys_CreateWindow, Sys_DestroyWindow, -1 },
	{ "Job System", E_InitJobSystem, E_TermJobSystem, -1 },
	{ "I/O System", E_InitIOSystem, E_TermIOSystem, NEP_LOAD_PRE_IO },
	{ "Event System", E_InitEventSystem, E_TermEventSystem, -1 },
	{ "Scripting", Sc_InitScriptSystem, Sc_TermScriptSystem, NEP_LOAD_PRE_SCRIPTING },
	{ "Components", E_InitComponents, E_TermComponents, NEP_LOAD_PRE_ECS },
	{ "Entities", E_InitEntities, E_TermEntities, -1},
	{ "ECSystems", E_InitECSystems, E_TermECSystems, -1 },
	{ "Resource System", E_InitResourceSystem, E_TermResourceSystem, NEP_LOAD_PRE_RESOURCES },
	{ "Render System", Re_InitRender, Re_TermRender, NEP_LOAD_PRE_RENDER },
	{ "Audio System", Au_Init, Au_Term, NEP_LOAD_PRE_AUDIO },
	{ "Resource Purge", NULL, E_PurgeResources, -1 },
	{ "Input", In_InitInput, In_TermInput, NEP_LOAD_PRE_INPUT },
	{ "UI", UI_InitUI, UI_TermUI, -1 },
	{ "Console", E_InitConsole, E_TermConsole, -1 },
	{ "Network", Net_Init, Net_Term, NEP_LOAD_PRE_NETWORK }
};
static const int32_t _subsystemCount = sizeof(_subsystems) / sizeof(_subsystems[0]);

bool
E_Init(int argc, char *argv[])
{
	int opt;
	const char *configFile = E_CONFIG_FILE;
	const char *logFile = NULL;
	const char *dataDir = NULL;
	bool waitForDebugger = false;

	while ((opt = getopt(argc, argv, "c:d:l:w")) != -1) {
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
		case 'w':
			waitForDebugger = true;
			break;
		}
	}

	srand((unsigned int)time(NULL));

	E_InitConfig(configFile);

	Sys_Init();
	Sys_InitMemory();
	Sys_InitLog(logFile);

	if (waitForDebugger)
		Sys_MessageBox("Waiting for Debugger", "Attach the debugger and click OK", MSG_ICON_INFO);

	if (dataDir)
		E_SetCVarStr("Engine_DataDir", dataDir);

	//__MathDbg_SanityTest();

	E_screenWidth = &E_GetCVarU32("Engine_ScreenWidth", 1280)->u32;
	E_screenHeight = &E_GetCVarU32("Engine_ScreenHeight", 853)->u32;

	if (App_applicationInfo.version.revision)
		Sys_LogEntry(EMOD, LOG_INFORMATION, "%s v%d.%d.%d.%d", App_applicationInfo.name, App_applicationInfo.version.major,
			App_applicationInfo.version.minor, App_applicationInfo.version.build, App_applicationInfo.version.revision);
	else
		Sys_LogEntry(EMOD, LOG_INFORMATION, "%s v%d.%d.%d", App_applicationInfo.name, App_applicationInfo.version.major,
			App_applicationInfo.version.minor, App_applicationInfo.version.build);
	Sys_LogEntry(EMOD, LOG_INFORMATION, "Copyright (C) %s", App_applicationInfo.copyright);

	Sys_LogEntry(EMOD, LOG_INFORMATION, "%s \"%s\" v%s", E_PGM_NAME, E_CODENAME, E_VER_STR);
	Sys_LogEntry(EMOD, LOG_INFORMATION, "Copyright (C) %s", E_CPY_STR);
	Sys_LogEntry(EMOD, LOG_INFORMATION, "Starting up...");

	Sys_LogEntry(EMOD, LOG_INFORMATION, "Host: %s", Sys_Hostname());
	Sys_LogEntry(EMOD, LOG_INFORMATION, "Platform: %s %s", Sys_OperatingSystem(), Sys_OperatingSystemVersionString());
	Sys_LogEntry(EMOD, LOG_INFORMATION, "CPU: %s", Sys_CpuName());
	Sys_LogEntry(EMOD, LOG_INFORMATION, "\tFrequency: %d MHz", Sys_CpuFreq());
	Sys_LogEntry(EMOD, LOG_INFORMATION, "\tCount: %d / %d", Sys_CpuCount(), Sys_CpuThreadCount());
	Sys_LogEntry(EMOD, LOG_INFORMATION, "\tArchitecture: %s", Sys_Machine());
	Sys_LogEntry(EMOD, LOG_INFORMATION, "\tBig Endian: %s", Sys_BigEndian() ? "yes" : "no");

	if (!E_InitPluginList()) {
		Sys_LogEntry(EMOD, LOG_CRITICAL, "Failed to load plugin list");
		return false;
	}

	E_LoadPlugins(NEP_LOAD_EARLY_INIT);

	for (int32_t i = 0; i < _subsystemCount; ++i) {
		if (_subsystems[i].loadPlugins != -1)
			E_LoadPlugins(_subsystems[i].loadPlugins);

		if (!_subsystems[i].init)
			continue;

		if (!_subsystems[i].init()) {
			char *msg = Sys_Alloc(sizeof(*msg), 512, MH_Transient);
			snprintf(msg, 512, "Failed to initialize %s. The program will now exit.", _subsystems[i].name);
			Sys_MessageBox("Fatal Error", msg, MSG_ICON_ERROR);

			Sys_LogEntry(EMOD, LOG_CRITICAL, "Failed to initialize %s", _subsystems[i].name);
			return false;
		}
	}

#ifdef _DEBUG
	char titleBuff[256];

	snprintf(titleBuff, sizeof(titleBuff), "%s v%u.%u.%u", App_applicationInfo.name,
		App_applicationInfo.version.major, App_applicationInfo.version.minor, App_applicationInfo.version.build);

	if (App_applicationInfo.version.revision)
		snprintf(titleBuff + strnlen(titleBuff, sizeof(titleBuff)), sizeof(titleBuff) - strnlen(titleBuff, sizeof(titleBuff)),
			".%u", App_applicationInfo.version.revision);

	snprintf(titleBuff + strnlen(titleBuff, sizeof(titleBuff)), sizeof(titleBuff) - strnlen(titleBuff, sizeof(titleBuff)),
		" - NekoEngine v%u.%u.%u", E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD);

	if (E_VER_REVISION)
		snprintf(titleBuff + strnlen(titleBuff, sizeof(titleBuff)), sizeof(titleBuff) / strnlen(titleBuff, sizeof(titleBuff)),
			".%u", E_VER_REVISION);

	snprintf(titleBuff + strnlen(titleBuff, sizeof(titleBuff)), sizeof(titleBuff) - strnlen(titleBuff, sizeof(titleBuff)),
		" - GPU: %s (%s)", Re_deviceInfo.deviceName, Re_backendName);

	Sys_SetWindowTitle(titleBuff);
#else
	Sys_SetWindowTitle(App_applicationInfo.name);
#endif

	_startTime = (double)Sys_Time();
	_frameLimiter = &E_GetCVarI32("Engine_FrameLimiter", 0)->i32;

	Sys_LogEntry(EMOD, LOG_INFORMATION, "Engine start up complete.");

	E_LoadPlugins(NEP_LOAD_POST_INIT);

	if (!App_InitApplication(argc, argv))
		return false;

	Sys_LogEntry(EMOD, LOG_INFORMATION, "Application started.");

	E_LoadPlugins(NEP_LOAD_POST_APP_INIT);

	Sys_ResetHeap(MH_Transient);

	lua_State *vm = Sc_CreateVM();
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

		if (!E_ProcessXrEvents()) {
			_shutdown = true;
			break;
		}

		if (Sys_ScreenVisible())
			E_Frame();

	//	Prof_Reset();
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

	Prof_BeginRegion("Logic", 1.f, 1.f, 1.f);

	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_LOGIC);
	E_ProcessEvents();
	Prof_EndRegion();

	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_POST_LOGIC);

	E_DrawConsole();

	Prof_BeginRegion("Render", 1.f, 1.f, 1.f);

	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_PRE_RENDER);
	Prof_InsertMarker("PreRender");

	Re_RenderFrame();
	E_XrPresent();

	Prof_InsertMarker("RenderFrame");
	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_POST_RENDER);
	Prof_EndRegion();

	In_Update();

	Prof_BeginRegion("Application", 1.f, 1.f, 1.f);
	App_Frame();
	Prof_EndRegion();

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
		Re_ScreenResized(Re_swapchain);

	E_Broadcast(EVT_SCREEN_RESIZED, NULL);
}

void
E_Term(void)
{
	Sys_LogEntry(EMOD, LOG_INFORMATION, "Shutting down...");

	Re_WaitIdle();

	App_TermApplication();

	if (Scn_activeScene)
		Scn_UnloadScene(Scn_activeScene);

	for (int32_t i = _subsystemCount - 1; i >= 0; --i) {
		if (!_subsystems[i].term)
			continue;

		_subsystems[i].term();

		if (_subsystems[i].loadPlugins != -1)
			E_UnloadPlugins(_subsystems[i].loadPlugins);
	}

	E_TermPluginList();

	Sys_LogMemoryStatistics();
	Sys_LogEntry(EMOD, LOG_INFORMATION, "Shut down complete.");

	E_TermConfig();

	Sys_TermLog();
	Sys_TermMemory();
	Sys_Term();
}
