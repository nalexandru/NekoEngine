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

#include <runtime/runtime.h>

#include <scene/camera.h>
#include <graphics/graphics.h>

struct ne_camera *ne_main_camera = NULL;

ne_status
cam_init(struct ne_camera *cam)
{
	const char *args[] =
	{
		"cam_ptr",
		cam,
		NULL
	};

	if (!cam)
		return NE_INVALID_ARGS;

	memset(cam, 0x0, sizeof(*cam));

	kmVec3Fill(&cam->pos, 0.f, 0.f, 3.f);

	kmVec3Fill(&cam->fwd, 0.f, 0.f, -1.f);
	kmVec3Fill(&cam->up, 0.f, 1.f, 1.f);
	kmVec3Fill(&cam->right, 1.f, 0.f, 0.f);
	kmVec3Fill(&cam->world_up, 0.f, 1.f, 0.f);
	kmQuaternionIdentity(&cam->rot);

	cam->yaw = -90.f;
	cam->pitch = 0.f;

	cam->near = .3f;
	cam->far = 1000.f;
	cam->fov = 90.f;
	cam->proj_type = PROJ_PERSPECTIVE;

	cam_update_view(cam);
	cam_update_proj(cam);

	cam->handle = comp_create(CAMERA_COMP_TYPE, 0, args);

	return cam->handle == NE_INVALID_COMPONENT ? NE_FAIL : NE_OK;
}

void
cam_draw_sky(struct ne_camera *cam,
	bool draw)
{
	//
}

void
cam_look_at(struct ne_camera *cam,
	kmVec3 *eye,
	kmVec3 *center,
	kmVec3 *up)
{
	kmMat4LookAt(&cam->view, eye, center, up);
	cam->proj_type = PROJ_PERSPECTIVE;
}

void
cam_update_proj(struct ne_camera *cam)
{
	if (cam->proj_type == PROJ_PERSPECTIVE)
		kmMat4PerspectiveProjection(&cam->proj, cam->fov / 2.f,
			(float)ne_gfx_screen_width / (float)ne_gfx_screen_height,
			cam->near, cam->far);
	else
		kmMat4OrthographicProjection(&cam->proj, 0.f,
			(float)ne_gfx_screen_width, (float)ne_gfx_screen_height,
			0.f, cam->near, cam->far);

	kmMat4 tmp;
	tmp.mat[0] = 1.f;
	tmp.mat[1] = 0.f;
	tmp.mat[2] = 0.f;
	tmp.mat[3] = 0.f;

	tmp.mat[4] = 0.f;
	tmp.mat[5] = -1.f;
	tmp.mat[6] = 0.f;
	tmp.mat[7] = 0.f;

	tmp.mat[8] = 0.f;
	tmp.mat[9] = 0.f;
	tmp.mat[10] = .5f;
	tmp.mat[11] = 0.f;

	tmp.mat[12] = 0.f;
	tmp.mat[13] = 0.f;
	tmp.mat[14] = .5f;
	tmp.mat[15] = 1.f;

	kmMat4Multiply(&cam->proj, &cam->proj, &tmp);
}

void
cam_update_view(struct ne_camera *cam)
{
	kmVec3 fwd;
	kmVec3 tmp;

	fwd.x = cosf(kmDegreesToRadians(cam->yaw)) *
		cosf(kmDegreesToRadians(cam->pitch));
	fwd.y = sinf(kmDegreesToRadians(cam->pitch));
	fwd.z = sinf(kmDegreesToRadians(cam->yaw)) *
		cosf(kmDegreesToRadians(cam->pitch));
	kmVec3Normalize(&cam->fwd, &fwd);

	kmVec3Normalize(&cam->right,
		kmVec3Cross(&fwd, &cam->fwd, &cam->world_up));
	kmVec3Normalize(&cam->up,
		kmVec3Cross(&fwd, &cam->right, &cam->fwd));

	kmMat4LookAt(&cam->view, &cam->pos,
		kmVec3Add(&tmp, &cam->pos, &cam->fwd), &cam->up);

	//kmMat4Transpose(&cam->view, &cam->view);

/*	kmMat4RotationQuaternion(&cam->view, &cam->rot);
	kmMat4Multiply(&cam->view, &cam->view, kmMat4Translation(&tmp, cam->pos.x, cam->pos.y, cam->pos.z));*/
}

void
cam_translate(
	struct ne_camera *cam,
	kmVec3 *v)
{
	kmVec3Add(&cam->pos, &cam->pos, v);
}

void
cam_rotate(
	struct ne_camera *cam,
	kmVec3 *axis,
	float angle)
{
//	kmVec3 *current;
//	kmQuaternionToAxisAngle(&cam->rot, &)
}

void
cam_forward(
	struct ne_camera *cam,
	float d)
{
	kmVec3 v;
	v.x = v.y = v.z = d;

	kmVec3Mul(&v, &cam->fwd, &v);
	kmVec3Add(&cam->pos, &cam->pos, &v);
}

void
cam_right(
	struct ne_camera *cam,
	float d)
{
	kmVec3 v;
	v.x = v.y = v.z = d;

	kmVec3Mul(&v, &cam->right, &v);
	kmVec3Add(&cam->pos, &cam->pos, &v);
}

void
cam_up(
	struct ne_camera *cam,
	float d)
{
	kmVec3 v;
	v.x = v.y = v.z = d;

	kmVec3Mul(&v, &cam->up, &v);
	kmVec3Add(&cam->pos, &cam->pos, &v);
}

void
cam_rot_x(
	struct ne_camera *cam,
	float a)
{
	float v = kmQuaternionGetPitch(&cam->rot) + a;
	kmQuaternionRotationPitchYawRoll(&cam->rot,
		v,
		kmQuaternionGetYaw(&cam->rot),
		kmQuaternionGetRoll(&cam->rot));
}

void
cam_rot_y(
	struct ne_camera *cam,
	float a)
{
	float v = kmQuaternionGetYaw(&cam->rot) + a;
	kmQuaternionRotationPitchYawRoll(&cam->rot,
		kmQuaternionGetPitch(&cam->rot),
		v,
		kmQuaternionGetRoll(&cam->rot));
}

void
cam_rot_z(
	struct ne_camera *cam,
	float a)
{
	float v = kmQuaternionGetRoll(&cam->rot) + a;
	kmQuaternionRotationPitchYawRoll(&cam->rot,
		kmQuaternionGetPitch(&cam->rot),
		kmQuaternionGetYaw(&cam->rot),
		v);
}

void
cam_release(struct ne_camera *cam)
{
	//
}

// Component

ne_status
camera_comp_create(
	void *comp,
	const void **args)
{
	size_t len = 0;
	struct ne_camera_comp *c = comp;

	while (*args) {
		len = strlen(*args);

		if (!strncmp(*args, "cam_ptr", len))
			c->cam = *(++args);

		++args;
	}

	return NE_OK;
}

void
camera_comp_destroy(void *comp)
{
	//
}

// Systems

void
camera_proj_system(
	double dt,
	void **comp)
{
	struct ne_camera_comp *c = comp[0];

	cam_update_proj(c->cam);
}

void
camera_view_system(
	double dt,
	void **comp)
{
	struct ne_camera_comp *c = comp[0];

	cam_update_view(c->cam);
}
