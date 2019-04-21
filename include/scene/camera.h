/* NekoEngine
 *
 * camera.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Camera
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _NE_SCENE_CAMERA_H_
#define _NE_SCENE_CAMERA_H_

#include <stdbool.h>

#include <system/platform.h>
#include <runtime/runtime.h>

#include <engine/math.h>
#include <engine/status.h>
#include <engine/components.h>
#include <scene/frustum.h>
#include <ecs/component.h>

typedef enum ne_projection_type
{
	PROJ_PERSPECTIVE = 0,
	PROJ_ORTOGRAPHIC = 1
} ne_projection_type;

struct ne_camera
{
	kmVec3 pos;
	kmMat4 view;
	kmMat4 proj;

	kmQuaternion rot;

	ne_projection_type proj_type;
	ne_frustum frustum;
	comp_handle handle;

	//

	kmVec3 fwd;
	kmVec3 up;
	kmVec3 right;
	kmVec3 world_up;

	//

	
	float yaw;
	float pitch;

	

	float near;
	float far;
	float fov;

	/*float view_distance;
	float fog_distance;
	kmVec3 fog_color;*/

	
} ne_camera;

struct ne_camera_comp
{
	NE_COMPONENT;

	struct ne_camera *cam;
};

ne_status	cam_init(struct ne_camera *cam);

void		cam_draw_sky(struct ne_camera *cam, bool draw);

void		cam_look_at(struct ne_camera *cam, kmVec3 *eye, kmVec3 *center, kmVec3 *up);
void		cam_update_proj(struct ne_camera *cam);
void		cam_update_view(struct ne_camera *cam);

void		cam_translate(struct ne_camera *cam, kmVec3 *v);
void		cam_rotate(struct ne_camera *cam, kmVec3 *axis, float angle);

void		cam_forward(struct ne_camera *cam, float d);
void		cam_right(struct ne_camera *cam, float d);
void		cam_up(struct ne_camera *cam, float d);

void		cam_rot_x(struct ne_camera *cam, float a);
void		cam_rot_y(struct ne_camera *cam, float a);
void		cam_rot_z(struct ne_camera *cam, float a);

void		cam_release(struct ne_camera *cam);

#ifdef _NE_ENGINE_INTERNAL_

ne_status	camera_comp_create(void *comp, const void **args);
void		camera_comp_destroy(void *comp);

void		camera_proj_system(double dt, void **comp);
void		camera_view_system(double dt, void **comp);

NE_REGISTER_COMPONENT(CAMERA_COMP_TYPE, struct ne_camera_comp, camera_comp_create, camera_comp_destroy)

#endif /* _NE_ENGINE_INTERNAL_ */

MIWA_EXPORT extern struct ne_camera *ne_main_camera;

#endif /* _NE_SCENE_CAMERA_H_ */

