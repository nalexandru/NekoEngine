#ifndef _NE_SCENE_CAMERA_H_
#define _NE_SCENE_CAMERA_H_

#include <Math/Math.h>
#include <Engine/Types.h>
#include <Engine/Component.h>

enum NeProjectionType
{
	PT_Perspective,
	PT_Orthographic
};

struct NeCamera
{
	COMPONENT_BASE;

	struct mat4 viewMatrix;
	struct mat4 projMatrix;

	struct vec3 rotation;
	float fov, zNear, zFar, aperture;
	enum NeProjectionType projection;
	
	uint64_t evt;
};

ENGINE_API extern struct NeCamera *Scn_activeCamera;

void Scn_ActivateCamera(struct NeCamera *cam);

#endif /* _NE_SCENE_CAMERA_H_ */
