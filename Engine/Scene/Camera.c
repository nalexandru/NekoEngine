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
	m4_ident(&cam->viewMatrix);

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

	m4_infinite_perspective_rz(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear);

	cam->evt = E_RegisterHandler(EVT_SCREEN_RESIZED, (NeEventHandlerProc)_RebuildProjection, cam);

	return true;
}

E_SYSTEM(SCN_UPDATE_CAMERA, ECSYS_GROUP_PRE_RENDER, ECSYS_PRI_CAM_VIEW, true, void, 2, TRANSFORM_COMP, CAMERA_COMP)
{
	struct NeTransform *xform = comp[0];
	struct NeCamera *cam = comp[1];

	struct mat4 m_rot, m_pos;
	struct quat q1, q2, rot;
	struct vec3 pos;

	quat_from_axis_angle(&q1, &v3_neg_x, cam->rotation.x);
	quat_from_axis_angle(&q2, &v3_neg_y, cam->rotation.y);

	quat_mul(&rot, &q1, &q2);
	quat_from_axis_angle(&q1, &v3_neg_z, cam->rotation.z);

	quat_mul(&rot, &rot, &q1);
	m4_rot_quat(&m_rot, &rot);

	xform_position(xform, &pos);
	m4_translate(&m_pos, pos.x, -pos.y, pos.z);

	m4_mul(&cam->viewMatrix, &m_rot, &m_pos);
}

static void
_TermCamera(struct NeCamera *cam)
{
	E_UnregisterHandler(cam->evt);
}

static void
_RebuildProjection(struct NeCamera *cam, void *args)
{
	m4_infinite_perspective_rz(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear);
}
