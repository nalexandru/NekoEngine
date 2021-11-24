#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>
#include <Engine/Job.h>
#include <Engine/Engine.h>
#include <Engine/Events.h>
#include <Engine/Version.h>
#include <Engine/ECSystem.h>
#include <Engine/Application.h>
#include <Input/Input.h>
#include <System/Log.h>
#include <UI/UI.h>

#ifndef _countof
#	define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

struct ApplicationInfo App_applicationInfo =
{
	L"NekoEngine Test Application",
	E_CPY_STR,
	{ E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD, E_VER_REVISION }
};

static uint64_t _sceneLoadedEvt;

struct FPSController
{
	COMPONENT_BASE;

	float movementSpeed, hRotationSpeed, vRotationSpeed;
	uint32_t moveForward, moveRight, moveUp, rotateHorizontal, rotateVertical;
};

struct Statistics
{
	COMPONENT_BASE;

	uint32_t frames;
	double time;
	wchar_t fpsBuff[20];
	wchar_t ftBuff[30];
};

static bool App_InitFlyController(struct FPSController *comp, const void **args);
static void App_TermFlyController(struct FPSController *comp) { }
static void App_FlyController(void **comp, void *args);
static void App_SceneLoaded(void *user, void *args);

static bool App_InitStatistics(struct Statistics *comp, const void **args) { return true; }
static void App_TermStatistics(struct Statistics *comp) { }
static void App_DrawStatistics(void **comp, void *args);

static volatile uint64_t _jobStart;

static void
_SleepJob(int worker, void *args)
{
	Sys_Sleep(1);
	Sys_LogEntry(L"JOB", LOG_DEBUG, L"Worker %d completed", worker);
}

static void
_SleepCompleted(uint64_t id)
{
	Sys_LogEntry(L"JOB", LOG_DEBUG, L"Time: %f", ((double)Sys_Time() - (double)_jobStart) * 0.000000001);
}

bool
App_InitApplication(int argc, char *argv[])
{
	const wchar_t *comp[] = { TRANSFORM_COMP, CAMERA_COMP, L"FlyController" };

	E_RegisterComponent(L"FlyController", sizeof(struct FPSController), 1, (CompInitProc)App_InitFlyController, (CompTermProc)App_TermFlyController);
	E_RegisterSystem(L"App_FlyController", ECSYS_GROUP_LOGIC, comp, _countof(comp), (ECSysExecProc)App_FlyController, 0);

	comp[0] = UI_CONTEXT_COMP; comp[1] = L"Statistics";
	E_RegisterComponent(L"Statistics", sizeof(struct Statistics), 1, (CompInitProc)App_InitStatistics, (CompTermProc)App_TermStatistics);
	E_RegisterSystem(L"App_DrawStatistics", ECSYS_GROUP_LOGIC, comp, 2, (ECSysExecProc)App_DrawStatistics, 0);

	_sceneLoadedEvt = E_RegisterHandler(EVT_SCENE_LOADED, App_SceneLoaded, NULL);

//	Scn_StartSceneLoad("/Scenes/Anna.scn");
//	Scn_StartSceneLoad("/Scenes/Helmet.scn");
//	Scn_StartSceneLoad("/Scenes/Sphere.scn");
//	Scn_StartSceneLoad("/Scenes/Main.scn");
	Scn_StartSceneLoad("/Scenes/Terrain.scn");

	_jobStart = Sys_Time();
	E_DispatchJobs(E_JobWorkerThreads(), _SleepJob, NULL, _SleepCompleted);
	
	Sys_LogEntry(L"JOB", LOG_DEBUG, L"Dispatched %d workers", E_JobWorkerThreads());

	return true;
}

void
App_Frame(void)
{
}

void
App_TermApplication(void)
{
}

bool
App_InitFlyController(struct FPSController *comp, const void **args)
{
	comp->movementSpeed = 100.f;
	comp->hRotationSpeed = 250.f;
	comp->vRotationSpeed = 100.f;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "MovementSpeed", len))
			comp->movementSpeed = strtof(*(++args), NULL);
		else if (!strncmp(arg, "HRotationSpeed", len))
			comp->hRotationSpeed = strtof(*(++args), NULL);
		else if (!strncmp(arg, "VRotationSpeed", len))
			comp->vRotationSpeed = strtof(*(++args), NULL);
	}

	comp->moveForward = In_CreateMap(L"forward");
	comp->moveRight = In_CreateMap(L"lateral");
	comp->moveUp = In_CreateMap(L"vertical");
	comp->rotateHorizontal = In_CreateMap(L"rotlMap");
	comp->rotateVertical = In_CreateMap(L"rotvMap");

	return true;
}

void
App_FlyController(void **comp, void *args)
{
	struct Transform *xform = comp[0];
	struct Camera *cam = comp[1];
	struct FPSController *ctrl = comp[2];

	if (In_UnmappedButtonDown(BTN_KEY_ESCAPE, 0))
		E_Shutdown();

	if (In_UnmappedButtonDown(BTN_MOUSE_MMB, 0))
		In_EnableMouseAxis(true);
	else if (In_UnmappedButtonUp(BTN_MOUSE_MMB, 0))
		In_EnableMouseAxis(false);

	const float mvmt = ctrl->movementSpeed * (float)E_deltaTime;
	const float vRot = ctrl->vRotationSpeed * (float)E_deltaTime;
	const float hRot = ctrl->hRotationSpeed * (float)E_deltaTime;

	cam->rotation.x += In_Axis(ctrl->rotateVertical) * vRot;
	cam->rotation.y += In_Axis(ctrl->rotateHorizontal) * hRot;

	xform_rotate(xform, In_Axis(ctrl->rotateHorizontal) * hRot, &v3_neg_y);
	xform_update_orientation(xform);

	xform_rotate(xform, In_Axis(ctrl->rotateVertical) * vRot, &xform->right);
	xform_update_orientation(xform);

	xform_move_forward(xform, In_Axis(ctrl->moveForward) * mvmt);
	xform_move_right(xform, In_Axis(ctrl->moveRight) * mvmt);
	xform_move_up(xform, In_Axis(ctrl->moveUp) * mvmt);
}

void
App_SceneLoaded(void *user, void *args)
{
	Scn_ActivateScene((struct Scene *)args);
}

void
App_DrawStatistics(void **comp, void *args)
{
	struct UIContext *ctx = comp[0];
	struct Statistics *stats = comp[1];
	double delta = E_Time() - stats->time;

	++stats->frames;

	if (delta > 1.0) {
		double ft = (delta / (double)stats->frames) * 1000;

		swprintf(stats->fpsBuff, sizeof(stats->fpsBuff) / 2, L"FPS: %d", stats->frames);
		swprintf(stats->ftBuff, sizeof(stats->ftBuff) / 2, L"Frame Time: %.02f ms", ft);

		stats->time += delta;
		stats->frames = 0;
	}

	UI_DrawText(ctx, stats->fpsBuff, 0.f, 0.f, 20.f, NULL);
	UI_DrawText(ctx, stats->ftBuff, 0.f, 20.f, 20.f, NULL);
}
