#ifndef _SCN_CAMERA_H_
#define _SCN_CAMERA_H_

#include <Math/Math.h>
#include <Engine/Types.h>
#include <Engine/Component.h>

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
	
	uint64_t evt;
};

ENGINE_API extern struct Camera *Scn_activeCamera;

bool Scn_InitCamera(struct Camera *cam, const void **args);
void Scn_TermCamera(struct Camera *cam);

void Scn_ActivateCamera(struct Camera *cam);

void Scn_UpdateCamera(void **comp, void *args);

#endif /* _SCN_CAMERA_H_ */
