#include <Scene/Scene.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>

#include <Engine/Engine.h>
#include <Engine/Events.h>
#include <Engine/ECSystem.h>
#include <Engine/Application.h>

#include <Input/Input.h>

#include <System/Log.h>

#include <UI/UI.h>

#ifndef _countof
#	define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

struct ApplicationInfo App_ApplicationInfo =
{
	L"NekoEngine Test Application",
	L"2020 Alexandru Naiman. All rights reserved.",
	{ 0, 7, 20, 0 }
};

static uint64_t _sceneLoadedEvt;

struct PlayerMovement
{
	COMPONENT_BASE;

	float movementSpeed, rotationSpeed;
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

static bool App_InitPlayerMovement(struct PlayerMovement *comp, const void **args);
static void App_TermPlayerMovement(struct PlayerMovement *comp) { }
static void App_PlayerMovement(void **comp, void *args);
static void App_SceneLoaded(void *user, void *args);

static bool App_InitStatistics(struct PlayerMovement *comp, const void **args) { return true; }
static void App_TermStatistics(struct PlayerMovement *comp) { }
static void App_DrawStatistics(void **comp, void *args);

bool
App_InitApplication(int argc, char *argv[])
{
	const wchar_t *comp[] = { TRANSFORM_COMP, L"PlayerMovement" };

	E_RegisterComponent(L"PlayerMovement", sizeof(struct PlayerMovement), 1, (CompInitProc)App_InitPlayerMovement, (CompTermProc)App_TermPlayerMovement);
	E_RegisterSystem(L"App_PlayerMovement", ECSYS_GROUP_LOGIC, comp, _countof(comp), (ECSysExecProc)App_PlayerMovement, 0);

	comp[0] = UI_CONTEXT_COMP; comp[1] = L"Statistics";
	E_RegisterComponent(L"Statistics", sizeof(struct Statistics), 1, (CompInitProc)App_InitStatistics, (CompTermProc)App_TermStatistics);
	E_RegisterSystem(L"App_DrawStatistics", ECSYS_GROUP_LOGIC, comp, _countof(comp), (ECSysExecProc)App_DrawStatistics, 0);

	_sceneLoadedEvt = E_RegisterHandler(EVT_SCENE_LOADED, App_SceneLoaded, NULL);
	Scn_StartSceneLoad("/Scenes/Sponza.scn");

	return true;
}

void
App_TermApplication(void)
{
}

bool
App_InitPlayerMovement(struct PlayerMovement *comp, const void **args)
{
	comp->movementSpeed = 100.f;
	comp->rotationSpeed = 50.f;

	for (args; *args; ++args) {
		const char *arg = (const char *)*args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "MovementSpeed", len))
			comp->movementSpeed = strtof((const char *)*(++args), NULL);
		else if (!strncmp(arg, "RotationSpeed", len))
			comp->rotationSpeed = strtof((const char *)*(++args), NULL);
	}

	comp->moveForward = In_CreateMap(L"forward");
	comp->moveRight = In_CreateMap(L"lateral");
	comp->moveUp = In_CreateMap(L"vertical");
	comp->rotateHorizontal = In_CreateMap(L"rotlMap");
	comp->rotateVertical = In_CreateMap(L"rotvMap");

	return true;
}

void
App_PlayerMovement(void **comp, void *args)
{
	struct Transform *xform = (struct Transform *)comp[0];
	struct PlayerMovement *mvmt = (struct PlayerMovement *)comp[1];
	struct vec3 raxis = { 0.f, -1.f, 0.f };
	float xlate, rot;

	xlate = mvmt->movementSpeed * (float)E_DeltaTime;
	rot = mvmt->rotationSpeed * (float)E_DeltaTime;

	xform_rotate(xform, In_Axis(mvmt->rotateHorizontal) * rot, &raxis);

	v3_copy(&raxis, &xform->right);
	xform_rotate(xform, (In_Axis(mvmt->rotateVertical) * -1.f) * rot, &raxis);
	
	xform_move_forward(xform, In_Axis(mvmt->moveForward) * xlate);
	xform_move_right(xform, In_Axis(mvmt->moveRight) * xlate);
	xform_move_up(xform, In_Axis(mvmt->moveUp) * -xlate);
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

