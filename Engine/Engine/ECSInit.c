#include <stdbool.h>

#include <Engine/ECSystem.h>
#include <Engine/Component.h>

#include <Scene/Systems.h>
#include <Scene/Components.h>

#include <Scene/Camera.h>
#include <Scene/Transform.h>
#include <Render/ModelRender.h>

bool E_LoadComponents()
{
	E_RegisterComponent(MODEL_RENDER_COMP, sizeof(struct ModelRender), (CompInitProc)Re_InitModelRender, (CompTermProc)Re_TermModelRender);
	E_RegisterComponent(TRANSFORM_COMP, sizeof(struct Transform), (CompInitProc)Scn_InitTransform, (CompTermProc)Scn_TermTransform);
	E_RegisterComponent(CAMERA_COMP, sizeof(struct Camera), (CompInitProc)Scn_InitCamera, (CompTermProc)Scn_TermCamera);

	return true;
}

bool E_RegisterSystems()
{
	const wchar_t *comp[] = { TRANSFORM_COMP, CAMERA_COMP };
	E_RegisterSystem(SCN_UPDATE_TRANSFORM_SYS, ECSYS_GROUP_PRE_RENDER, comp, 1, (ECSysExecProc)Scn_UpdateTransform, 0);
	E_RegisterSystem(SCN_UPDATE_CAMERA_SYS, ECSYS_GROUP_PRE_RENDER, comp, 2, (ECSysExecProc)Scn_UpdateCamera, 0);

	return true;
}