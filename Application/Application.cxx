#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>
#include <Engine/Job.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Engine/Events.h>
#include <Engine/Version.h>
#include <Engine/ECSystem.h>
#include <Engine/Application.h>
#include <Input/Input.h>
#include <System/Log.h>
#include <UI/UI.h>

NE_APPLICATION("NekoEngine Test Application", E_CPY_STR, E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD, E_VER_REVISION);
NE_MAIN;

static uint64_t f_sceneLoadedEvt;

struct FlyController
{
	NE_COMPONENT_BASE;

	float movementSpeed, hRotationSpeed, vRotationSpeed;
	uint32_t moveForward, moveRight, moveUp, rotateHorizontal, rotateVertical;
};
#define FLY_CONTROLLER_COMP "FlyController"

struct Statistics
{
	NE_COMPONENT_BASE;

	uint32_t frames;
	double time;
	char fpsBuff[20];
	char ftBuff[30];
};
#define STATISTICS_COMP "Statistics"

static bool App_InitFlyController(struct FlyController *comp, const char **args);
static void App_TermFlyController(struct FlyController *comp) { }
static void App_FlyController(void **comp, void *args);
static void App_SceneLoaded(void *user, void *args);

static bool App_InitStatistics(struct Statistics *comp, const char **args) { return true; }
static void App_TermStatistics(struct Statistics *comp) { }
static void App_DrawStatistics(void **comp, void *args);

static volatile uint64_t f_jobStart;

NE_REGISTER_COMPONENT(FLY_CONTROLLER_COMP, struct FlyController, 1, App_InitFlyController, nullptr, App_TermFlyController);
NE_REGISTER_COMPONENT(STATISTICS_COMP, struct Statistics, 1, App_InitStatistics, nullptr, App_TermStatistics);

static void
SleepJob(int worker, void *args)
{
	Sys_Sleep(1);
	Sys_LogEntry("JOB", LOG_DEBUG, "Worker %d (id %d) completed", worker, E_WorkerId());
}

static void
SleepCompleted(uint64_t id, volatile bool *done)
{
	Sys_LogEntry("JOB", LOG_DEBUG, "Time: %f", ((double)Sys_Time() - (double)f_jobStart) * 0.000000001);
	*done = true;
}

bool
App_EarlyInit(int argc, char *argv[])
{
	return true;
}

bool
App_InitApplication(int argc, char *argv[])
{
	NeCompTypeId comp[] = { NE_TRANSFORM_ID, NE_CAMERA_ID, FLY_CONTROLLER_COMP_ID };
	E_RegisterSystemId("App_FlyController", ECSYS_GROUP_LOGIC_HASH, comp, NE_ARRAY_SIZE(comp), (NeECSysExecProc)App_FlyController, 0, true);

	comp[0] = NE_UI_CONTEXT_ID; comp[1] = STATISTICS_COMP_ID;
	E_RegisterSystemId("App_DrawStatistics", ECSYS_GROUP_LOGIC_HASH, comp, 2, (NeECSysExecProc)App_DrawStatistics, 0, true);

	f_sceneLoadedEvt = E_RegisterHandler(EVT_SCENE_LOADED, App_SceneLoaded, NULL);
	if (!CVAR_STRING("Scene_DefaultScene"))
		E_SetCVarStr("Scene_DefaultScene", "/Scenes/Sponza.scn");

	volatile bool jobCompleted = false;

	f_jobStart = Sys_Time();
	E_DispatchJobs(E_JobWorkerThreads(), SleepJob, NULL, (NeJobCompletedProc)SleepCompleted, (void *)&jobCompleted);

	Sys_LogEntry("JOB", LOG_DEBUG, "Dispatched %d workers", E_JobWorkerThreads());

	while (!jobCompleted) ;
	while (!jobCompleted) ;

	return true;
}

void
App_Frame(void)
{
}

void
App_TermApplication(void)
{
	E_UnregisterHandler(f_sceneLoadedEvt);
}

bool
App_InitFlyController(struct FlyController *comp, const char **args)
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

	comp->moveForward = In_CreateMap("forward");
	comp->moveRight = In_CreateMap("lateral");
	comp->moveUp = In_CreateMap("vertical");
	comp->rotateHorizontal = In_CreateMap("rotlMap");
	comp->rotateVertical = In_CreateMap("rotvMap");

	return true;
}

void
App_FlyController(void **comp, void *args)
{
	struct NeTransform *xform = (struct NeTransform *)comp[0];
	struct NeCamera *cam = (struct NeCamera *)comp[1];
	struct FlyController *ctrl = (struct FlyController *)comp[2];

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

	const struct NeVec3 negy = { 0.f, -1.f, 0.f };
	Xform_Rotate(xform, In_Axis(ctrl->rotateHorizontal) * hRot, &negy);
	Xform_UpdateOrientation(xform);

	Xform_Rotate(xform, In_Axis(ctrl->rotateVertical) * vRot, &xform->right);
	Xform_UpdateOrientation(xform);

	Xform_MoveForward(xform, In_Axis(ctrl->moveForward) * mvmt);
	Xform_MoveRight(xform, In_Axis(ctrl->moveRight) * mvmt);
	Xform_MoveUp(xform, In_Axis(ctrl->moveUp) * mvmt);
}

void
App_SceneLoaded(void *user, void *args)
{
	Scn_ActivateScene((struct NeScene *)args);
}

void
App_DrawStatistics(void **comp, void *args)
{
	struct NeUIContext *ctx = (struct NeUIContext *)comp[0];
	struct Statistics *stats = (struct Statistics *)comp[1];
	double delta = E_Time() - stats->time;

	++stats->frames;

	if (delta > 1.0) {
		double ft = (delta / (double)stats->frames) * 1000;

		(void)snprintf(stats->fpsBuff, sizeof(stats->fpsBuff), "FPS: %d", stats->frames);
		(void)snprintf(stats->ftBuff, sizeof(stats->ftBuff), "Frame Time: %.02f ms", ft);

		stats->time += delta;
		stats->frames = 0;
	}

	UI_DrawText(ctx, stats->fpsBuff, 0.f, 0.f, 20.f, NULL);
	UI_DrawText(ctx, stats->ftBuff, 0.f, 20.f, 20.f, NULL);

	char buff[256];
	snprintf(buff, 256, "Time: %f", E_Time());
	UI_DrawText(ctx, buff, 0.f, 40.f, 20.f, NULL);
}
