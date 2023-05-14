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
#include <Render/Render.h>
#include <Render/Backend.h>
#include <Render/Graph/Graph.h>
#include <Engine/XR.h>
#include <Engine/Event.h>
#include <Engine/Events.h>
#include <Engine/Console.h>
#include <Engine/Version.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Engine/Application.h>
#include <Animation/Animation.h>
#include <Network/Network.h>
#include <Script/Script.h>
#include <UI/UI.h>

#include "ECS.h"

#define EMOD	"Engine"

void *E_screen = NULL;
uint32_t *E_screenWidth = NULL;
uint32_t *E_screenHeight = NULL;
double E_deltaTime = 0.0;

static bool f_shutdown;
static double f_startTime, f_prevTime;
static int32_t *f_frameLimiter = NULL;

struct NeEngineSubsystem
{
	char name[256];
	bool (*init)(void);
	void (*term)(void);
	enum NePluginLoadOrder loadPlugins;
};

static struct NeEngineSubsystem f_subsystems[] =
{
	{ "Window", Sys_CreateWindow, Sys_DestroyWindow, -1 },
	{ "Job System", E_InitJobSystem, E_TermJobSystem, -1 },
	{ "I/O System", E_InitIOSystem, E_TermIOSystem, NEP_LOAD_PRE_IO },
	{ "Scripting", Sc_InitScriptSystem, Sc_TermScriptSystem, NEP_LOAD_PRE_SCRIPTING },
	{ "Components", E_InitComponents, E_TermComponents, NEP_LOAD_PRE_ECS },
	{ "Entities", E_InitEntities, E_TermEntities, -1},
	{ "ECSystems", E_InitECSystems, E_TermECSystems, -1 },
	{ "Resource System", E_InitResourceSystem, E_TermResourceSystem, NEP_LOAD_PRE_RESOURCES },
	{ "Render System", Re_InitRender, Re_TermRender, NEP_LOAD_PRE_RENDER },
	{ "Audio System", Au_Init, Au_Term, NEP_LOAD_PRE_AUDIO },
	{ "Resource Purge", NULL, E_PurgeResources, -1 },
	{ "Input", In_InitInput, In_TermInput, NEP_LOAD_PRE_INPUT },
	{ "UI", UI_InitUI, UI_TermUI, NEP_LOAD_PRE_UI },
	{ "Console", E_InitConsole, E_TermConsole, -1 },
	{ "Network", Net_Init, Net_Term, NEP_LOAD_PRE_NETWORK },
	{ "Animation System", Anim_InitAnimationSystem, Anim_TermAnimationSystem, -1 }
};

bool
E_Init(int argc, char *argv[])
{
	int opt;
	const char *configFile = NULL;
	const char *logFile = NULL;
	const char *dataDir = NULL;
	const char *scene = NULL;
	bool waitForDebugger = false;

#ifdef SYS_PLATFORM_WINDOWS
	bool forceDebugConsole = false;

	while ((opt = getopt(argc, argv, "c:d:l:s:wi")) != -1) {
#else
	while ((opt = getopt(argc, argv, "c:d:l:s:w")) != -1) {
#endif
		switch (opt) {
		case 'c': configFile = optarg; break;
		case 'd': dataDir = optarg; break;
		case 'l': logFile = optarg; break;
		case 's': scene = optarg; break;
		case 'w': waitForDebugger = true; break;
#ifdef SYS_PLATFORM_WINDOWS
		case 'i': forceDebugConsole = true; break;
#endif
		}
	}

	srand((unsigned int)time(NULL));

	E_InitConfig(configFile);

#ifdef SYS_PLATFORM_WINDOWS
	if (forceDebugConsole)
		E_SetCVarBln("Win32_ForceDebugConsole", true);
#endif

	Sys_Init();
	Sys_InitMemory();
	Sys_InitLog(logFile);

	if (waitForDebugger)
		Sys_MessageBox("Waiting for Debugger", "Attach the debugger and click OK", MSG_ICON_INFO);

	if (dataDir)
		E_SetCVarStr("Engine_DataDir", dataDir);

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
#ifdef SYS_BIG_ENDIAN
	Sys_LogEntry(EMOD, LOG_INFORMATION, "\tBig Endian: yes");
#else
	Sys_LogEntry(EMOD, LOG_INFORMATION, "\tBig Endian: no");
#endif
	Sys_LogEntry(EMOD, LOG_INFORMATION, "RAM: %llu MB", Sys_TotalMemory() / 1024 / 1024);

	if (!E_InitPluginList()) {
		Sys_LogEntry(EMOD, LOG_CRITICAL, "Failed to load plugin list");
		return false;
	}

	if (!E_InitEventSystem())
		return false;

	if (!E_LoadPlugins(NEP_LOAD_EARLY_INIT))
		return false;

	if (!App_EarlyInit(argc, argv))
		return false;

	for (int32_t i = 0; i < NE_ARRAY_SIZE(f_subsystems); ++i) {
		if (f_subsystems[i].loadPlugins != -1)
			if (!E_LoadPlugins(f_subsystems[i].loadPlugins))
				return false;

		if (!f_subsystems[i].init)
			continue;

		if (!f_subsystems[i].init()) {
			char *msg = Sys_Alloc(sizeof(*msg), 512, MH_Transient);
			snprintf(msg, 512, "Failed to initialize %s. The program will now exit.", f_subsystems[i].name);
			Sys_MessageBox("Fatal Error", msg, MSG_ICON_ERROR);

			Sys_LogEntry(EMOD, LOG_CRITICAL, "Failed to initialize %s", f_subsystems[i].name);
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

	f_startTime = (double)Sys_Time();
	f_frameLimiter = &E_GetCVarI32("Engine_FrameLimiter", 0)->i32;

	Sys_LogEntry(EMOD, LOG_INFORMATION, "Engine start up complete.");

	E_LoadPlugins(NEP_LOAD_POST_INIT);

	if (!App_InitApplication(argc, argv))
		return false;

	Sys_LogEntry(EMOD, LOG_INFORMATION, "Application started.");

	E_LoadPlugins(NEP_LOAD_POST_APP_INIT);

	E_ProcessEvents();
	Sys_ResetHeap(MH_Transient);

	E_ConsolePrint("NekoEngine v%s on %s %s %s", E_VER_STR, Sys_OperatingSystem(),
					Sys_OperatingSystemVersionString(), Sys_Machine());

	if (!Re_activeGraph)
		Re_activeGraph = Re_CreateDefaultGraph();

	lua_State *vm = Sc_CreateVM();
	Sc_LoadScriptFile(vm, "/Scripts/test.lua");
	Sc_DestroyVM(vm);

	if (scene) {
		Scn_StartSceneLoad(scene);
	} else {
		scene = CVAR_STRING("Scene_DefaultScene");
		if (scene)
			Scn_StartSceneLoad(scene);
	}

	return true;
}

int
E_Run(void)
{
	while (!f_shutdown) {
		if (!Sys_ProcessEvents()) {
			f_shutdown = true;
			break;
		}

		if (!E_ProcessXrEvents()) {
			f_shutdown = true;
			break;
		}

		if (!Sys_ScreenVisible())
			continue;
		
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

	E_deltaTime = now - f_prevTime;
	f_prevTime = now;

	if (!Scn_activeScene || Scn_activeScene->camera == NE_INVALID_HANDLE) {
		E_ProcessEvents();
		App_Frame();

		return;
	}

	E_ReloadSystemScripts();

	E_ProcessMessages(Scn_activeScene);

	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_LOGIC_HASH);
	E_ProcessEvents();

	E_ExecuteSystemGroupS(Scn_activeScene, ECSYS_GROUP_POST_LOGIC_HASH);
	Scn_Commit(Scn_activeScene);

	E_DistributeMessages();
	E_DrawConsole();

	Re_RenderFrame();
	E_XrPresent();

	In_Update();
	App_Frame();

	if (*f_frameLimiter)
		nextFrame = now + (1.0 / *f_frameLimiter);
}

double
E_Time(void)
{
	return ((double)Sys_Time() - f_startTime) * 1e-9;
}

void
E_Shutdown(void)
{
	f_shutdown = true;
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

	E_UnloadPlugins(NEP_LOAD_POST_APP_INIT);
	App_TermApplication();

	Scn_UnloadScenes();
	E_UnloadPlugins(NEP_LOAD_POST_INIT);

	for (int32_t i = NE_ARRAY_SIZE(f_subsystems) - 1; i >= 0; --i) {
		if (!f_subsystems[i].term)
			continue;

		f_subsystems[i].term();

		if (f_subsystems[i].loadPlugins != -1)
			E_UnloadPlugins(f_subsystems[i].loadPlugins);
	}

	E_TermEventSystem();
	E_TermPluginList();

	Sys_LogMemoryStatistics();
	Sys_LogEntry(EMOD, LOG_INFORMATION, "Shut down complete.");

	E_TermConfig();

	Sys_TermLog();
	Sys_TermMemory();
	Sys_Term();
}

/* NekoEngine
 *
 * Engine.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
