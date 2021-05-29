#include <Scene/Camera.h>
#include <Scene/Transform.h>
#include <Engine/Engine.h>
#include <Engine/Event.h>
#include <Engine/Events.h>

struct Camera *Scn_activeCamera = NULL;

static void _RebuildProjection(struct Camera *cam, void *args);

bool
Scn_InitCamera(struct Camera *cam, const void **args)
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

//	m4_infinite_perspective_rz(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear);

	m4_perspective(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear, cam->zFar);

	cam->evt = E_RegisterHandler(EVT_SCREEN_RESIZED, (EventHandlerProc)_RebuildProjection, cam);

	return true;
}

void
Scn_TermCamera(struct Camera *cam)
{
	E_UnregisterHandler(cam->evt);
}

void
Scn_ActivateCamera(struct Camera *cam)
{
	Scn_activeCamera = cam;
}

void
Scn_UpdateCamera(void **comp, void *args)
{
	struct Transform *xform = comp[0];
	struct Camera *cam = comp[1];

	struct mat4 m_rot, m_pos;
	struct quat rot;
	struct vec3 pos;

	xform_rotation(xform, &rot);
	m4_rot_quat(&m_rot, &rot);

	xform_position(xform, &pos);
	m4_translate(&m_pos, pos.x, -pos.y, pos.z);

	m4_mul(&cam->viewMatrix, &m_rot, &m_pos);
}

static void
_RebuildProjection(struct Camera *cam, void *args)
{
	m4_infinite_perspective_rz(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear);
	m4_perspective(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear, cam->zFar);
}
