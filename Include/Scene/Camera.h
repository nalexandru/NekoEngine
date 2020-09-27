#ifndef _SCN_CAMERA_H_
#define _SCN_CAMERA_H_

#include <Math/Math.h>
#include <Engine/Component.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ProjectionType
{
	PT_Perspective,
	PT_Orthographic
};

struct Camera
{
	COMPONENT_BASE;

	struct mat4 viewMatrix;
	struct mat4 projMatrix;

	float fov, zNear, zFar, aperture;
	enum ProjectionType projection;
};

extern struct Camera *Scn_ActiveCamera;

bool Scn_InitCamera(struct Camera *cam, const void **args);
void Scn_TermCamera(struct Camera *cam);

void Scn_ActivateCamera(struct Camera *cam);

void Scn_UpdateCamera(void **comp, void *args);

#ifdef __cplusplus
}
#endif

#endif /* _SCN_CAMERA_H_ */
