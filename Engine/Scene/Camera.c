#include <Scene/Camera.h>
#include <Scene/Systems.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>
#include <Engine/Engine.h>
#include <Engine/Event.h>
#include <Engine/Events.h>

struct NeCamera *Scn_activeCamera = NULL;

static bool _InitCamera(struct NeCamera *cam, const void **args);
static void _TermCamera(struct NeCamera *cam);
static void _RebuildProjection(struct NeCamera *cam, void *args);

E_REGISTER_COMPONENT(CAMERA_COMP, struct NeCamera, 16, _InitCamera, _TermCamera)

void
Scn_ActivateCamera(struct NeCamera *cam)
{
	Scn_activeCamera = cam;
}

static bool
_InitCamera(struct NeCamera *cam, const void **args)
{
	M_MatrixIdentity(&cam->viewMatrix);

	cam->fov = 90.f;
	cam->zNear = .1f;
	cam->zFar = 1024.f;
	cam->aperture = 0.0016f;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Active", len)) {
			if (!strncmp(*(++args), "true", 4))
				Scn_activeCamera = cam;
		} else if (!strncmp(arg, "Fov", len)) {
			cam->fov = (float)atof(*(++args));
		} else if (!strncmp(arg, "Near", len)) {
			cam->zNear = (float)atof(*(++args));
		} else if (!strncmp(arg, "Far", len)) {
			cam->zFar = (float)atof(*(++args));
		} else if (!strncmp(arg, "Aperture", len)) {
			cam->zFar = (float)atof(*(++args));
		}
	}

	M_InfinitePerspectiveMatrixRZ(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear);

	cam->evt = E_RegisterHandler(EVT_SCREEN_RESIZED, (NeEventHandlerProc)_RebuildProjection, cam);

	return true;
}

E_SYSTEM(SCN_UPDATE_CAMERA, ECSYS_GROUP_PRE_RENDER, ECSYS_PRI_CAM_VIEW, true, void, 2, TRANSFORM_COMP, CAMERA_COMP)
{
	struct NeTransform *xform = comp[0];
	struct NeCamera *cam = comp[1];

	struct NeVec3 pos, front;
	xform_position(xform, &pos);
	M_MatrixLookAt(&cam->viewMatrix, &pos, M_AddVec3(&front, &pos, &xform->forward), &xform->up);
}

static void
_TermCamera(struct NeCamera *cam)
{
	E_UnregisterHandler(cam->evt);
}

static void
_RebuildProjection(struct NeCamera *cam, void *args)
{
	//M_InfinitePerspectiveMatrix(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zFar, cam->zNear);
	M_InfinitePerspectiveMatrixRZ(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear);
}
