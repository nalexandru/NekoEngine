/* NekoEngine
 *
 * systems.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Test Application
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

#include <stdio.h>

#include <gui/gui.h>
#include <system/log.h>
#include <scene/camera.h>
#include <engine/input.h>
#include <engine/engine.h>
#include <graphics/graphics.h>

#include "systems.h"
#include "components.h"

static char _fps_buff[20] = { 'F', 'P', 'S', 0x0 };
static char _ft_buff[30] = { 'F', 'r', 'a', 'm', 'e', ' ', 'T', 'i', 'm', 'e', 0x0 };

void
fps_system(
	double dt,
	void **comp)
{
	struct fps_component *f = comp[0];
	double time = engine_get_time();
	double delta = time - f->time;

	++f->frames;

	if (delta > 1.f) {
		float ft = (delta / (double)f->frames) * 1000;

		snprintf(_fps_buff, 20, "FPS: %d", f->frames);
		snprintf(_ft_buff, 30, "Frame Time: %.02f ms", ft);

		f->time += delta;
		f->frames = 0;
	}

	gui_draw_text(_fps_buff, 5.f, 5.f, ne_system_font);
	gui_draw_text(_ft_buff, 5.f, 25.f, ne_system_font);
}

void
dbg_system(
	double dt,
	void **comp)
{
	char buff[256];

	snprintf(buff, 256, "Camera: %.02f, %.02f, %.02f",
		ne_main_camera->pos.x, ne_main_camera->pos.y,
		ne_main_camera->pos.z);

	gui_draw_text(buff, 5.f, 100.f, ne_system_font);
}

void
movement_system(
	double dt,
	void **comp)
{
	struct ne_camera_comp *cam_comp = comp[0];
	struct movement_component *mov = comp[1];
	kmVec3 *pos = &cam_comp->cam->pos;
	float speed = 10.f;
	float velocity = speed * dt;

	kmVec3 v_vec, tmp;
	kmVec3Fill(&v_vec, velocity, velocity, velocity);

	if (input_get_key(NE_KEY_W))
		kmVec3Add(&cam_comp->cam->pos, &cam_comp->cam->pos,
			kmVec3Mul(&tmp, &cam_comp->cam->fwd, &v_vec));
	else if (input_get_key(NE_KEY_S))
		kmVec3Subtract(&cam_comp->cam->pos, &cam_comp->cam->pos,
			kmVec3Mul(&tmp, &cam_comp->cam->fwd, &v_vec));

	if (input_get_key(NE_KEY_A))
		kmVec3Subtract(&cam_comp->cam->pos, &cam_comp->cam->pos,
			kmVec3Mul(&tmp, &cam_comp->cam->right, &v_vec));
	else if (input_get_key(NE_KEY_D))
		kmVec3Add(&cam_comp->cam->pos, &cam_comp->cam->pos,
			kmVec3Mul(&tmp, &cam_comp->cam->right, &v_vec));

	if (input_get_key(NE_KEY_Q))
		cam_comp->cam->yaw -= velocity * 10;
	else if (input_get_key(NE_KEY_E))
		cam_comp->cam->yaw += velocity * 10;



	/*float r = 10.f;
	mov->light->pos.x = sin(engine_get_time()) * r;
	mov->light->pos.z = cos(engine_get_time()) * r;*/

//	memcpy(&mov->light->pos,
//		&cam_comp->cam->pos, sizeof(kmVec3));
}

void
camera_system(
	double dt,
	void **comp)
{
	struct ne_camera_comp *cam_comp = comp[0];
	struct movement_component *mov = comp[1];
	kmVec3 *pos;
	float speed = 7.f;
	float velocity = speed * dt;

	pos = &cam_comp->cam->pos;

	if (mov->positive) {
		pos->x += velocity;
		if (cam_comp->cam->pos.x > 5.f)
			mov->positive = false;
	} else {
		pos->x -= velocity;
		if (cam_comp->cam->pos.x < -5.f)
			mov->positive = true;
	}
}
