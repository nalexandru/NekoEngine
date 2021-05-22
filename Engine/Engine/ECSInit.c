#include <stdbool.h>

#include <Engine/ECSystem.h>
#include <Engine/Component.h>

#include <Scene/Systems.h>
#include <Scene/Components.h>

#include <Scene/Camera.h>
#include <Scene/Transform.h>

#include <Render/Systems.h>

#include <Render/Components/ModelRender.h>

#include <UI/UI.h>

bool E_LoadComponents(void)
{
	E_RegisterComponent(MODEL_RENDER_COMP, sizeof(struct ModelRender), 1, (CompInitProc)Re_InitModelRender, (CompTermProc)Re_TermModelRender);
	E_RegisterComponent(TRANSFORM_COMP, sizeof(struct Transform), 16, (CompInitProc)Scn_InitTransform, (CompTermProc)Scn_TermTransform);
	E_RegisterComponent(CAMERA_COMP, sizeof(struct Camera), 16, (CompInitProc)Scn_InitCamera, (CompTermProc)Scn_TermCamera);
	E_RegisterComponent(UI_CONTEXT_COMP, sizeof(struct UIContext), 1, (CompInitProc)UI_InitContext, (CompTermProc)UI_TermContext);

	return true;
}

bool E_RegisterSystems(void)
{
	const wchar_t *comp[] = { TRANSFORM_COMP, CAMERA_COMP };
	E_RegisterSystem(SCN_UPDATE_TRANSFORM, ECSYS_GROUP_PRE_RENDER, comp, 1, (ECSysExecProc)Scn_UpdateTransform, 0);
	E_RegisterSystem(SCN_UPDATE_CAMERA, ECSYS_GROUP_PRE_RENDER, comp, 2, (ECSysExecProc)Scn_UpdateCamera, 0);

	comp[1] = MODEL_RENDER_COMP;
	E_RegisterSystem(RE_COLLECT_DRAWABLES, ECSYS_GROUP_MANUAL, comp, 2, (ECSysExecProc)Re_CollectDrawables, 0);

	comp[0] = UI_CONTEXT_COMP;
	E_RegisterSystem(UI_RESET_CONTEXT, ECSYS_GROUP_POST_RENDER, comp, 1, (ECSysExecProc)UI_ResetContext, 0);

	return true;
}
