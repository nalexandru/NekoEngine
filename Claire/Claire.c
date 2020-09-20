#include <Scene/Scene.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>

#include <Engine/Engine.h>
#include <Engine/Events.h>
#include <Engine/ECSystem.h>
#include <Engine/Application.h>

#include <Input/Input.h>

#include <System/Log.h>

#ifndef _countof
#	define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

struct ApplicationInfo App_ApplicationInfo =
{
	L"NekoEngine Test Application",
	{ 0, 7, 20, 0 }
};

static uint64_t _sceneLoadedEvt;

struct PlayerMovement
{
	COMPONENT_BASE;

	float movementSpeed, rotationSpeed;
	uint32_t moveForward, moveRight, moveUp, rotateHorizontal, rotateVertical;
};

static bool App_InitPlayerMovement(struct PlayerMovement *comp, const void **args);
static void App_TermPlayerMovement(struct PlayerMovement *comp) { }
static void App_PlayerMovement(void **comp, void *args);
static void App_SceneLoaded(void *user, void *args);

bool
App_InitApplication(int argc, char *argv[])
{
	const wchar_t *comp[] = { TRANSFORM_COMP, L"PlayerMovement" };

	E_RegisterComponent(L"PlayerMovement", sizeof(struct PlayerMovement), (CompInitProc)App_InitPlayerMovement, (CompTermProc)App_TermPlayerMovement);
	E_RegisterSystem(L"App_PlayerMovement", ECSYS_GROUP_PRE_RENDER, comp, _countof(comp), (ECSysExecProc)App_PlayerMovement, 0);

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

	v3(&raxis, 1.f, 0.f, 0.f);
	xform_rotate(xform, In_Axis(mvmt->rotateVertical) * rot, &raxis);

	xform_move_forward(xform, In_Axis(mvmt->moveForward) * xlate);
	xform_move_right(xform, In_Axis(mvmt->moveRight) * xlate);
	xform_move_up(xform, In_Axis(mvmt->moveUp) * -xlate);
}

void
App_SceneLoaded(void *user, void *args)
{
	Scn_ActivateScene((struct Scene *)args);
}

