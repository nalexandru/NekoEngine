/* NekoEngine
 *
 * test_app.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Application Interface
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

#include "test_app.h"

#include "systems.h"
#include "components.h"

#include <gui/gui.h>
#include <graphics/texture.h>
#include <graphics/graphics.h>
#include <engine/components.h>
#include <engine/resource.h>
#include <graphics/mesh.h>
#include <scene/camera.h>
#include <scene/transform.h>
#include <ecs/ecsys.h>
#include <ecs/entity.h>
#include <system/log.h>
#include <graphics/graphics.h>
#include <engine/input.h>

ne_app_module _test_app_module =
{
	NE_APP_API_VER,
	tapp_init,
	tapp_update,
	tapp_draw,
	tapp_screen_resized,
	tapp_destroy
};

const ne_app_module *
create_app_module(void)
{
	return &_test_app_module;
}

ne_widget *h_w, *j_w, *k_w, *l_w;
rt_array widgets;
comp_handle drawable_comp;
struct ne_camera tapp_camera;

static inline void
_widgets(void)
{
	void *p = NULL;

	for (uint32_t i = 0; i < 12; ++i) {
		p = gui_widget_create(i * 100, 420, 100, 100);
		rt_array_add_ptr(&widgets, p);

		p = gui_widget_create(i * 100, 520, 100, 100);
		rt_array_add_ptr(&widgets, p);

		p = gui_widget_create(i * 100, 620, 100, 100);
		rt_array_add_ptr(&widgets, p);

		p = gui_widget_create(i * 100, 0, 100, 100);
		rt_array_add_ptr(&widgets, p);
	}
}

static inline void
_kat(void)
{
	const char* args[] =
	{
		"mesh",
		"/meshes/kat.nemesh",

		"mat",
		"/materials/kat/hair.nemat",

		"mat",
		"/materials/kat/gears_and_cloth.nemat",

		"mat",
		"/materials/kat/skin_and_jumper.nemat",

		"mat",
		"/materials/kat/skin_and_jumper.nemat",

		"mat",
		"/materials/kat/gears_and_cloth.nemat",

		"mat",
		"/materials/kat/eye.nemat",

		"mat",
		"/materials/kat/face.nemat",

		"mat",
		"/materials/kat/caruncles.nemat",

		"mat",
		"/materials/kat/gears_and_cloth.nemat",

		NULL
	};

	struct ne_entity_comp_info comps[2] =
	{
		{ DRAWABLE_MESH_COMP_TYPE, args },
		{ TRANSFORM_COMP_TYPE, NULL },
	};

	entity_handle mesh = entity_create_v(2, comps);

	struct ne_transform_comp* t = entity_get_component(mesh,
		comp_get_type_id(TRANSFORM_COMP_TYPE));

	kmVec3Fill(&t->pos,
		0.f,
		-4.f,
		0.f);
}

static inline void
_pbr_spheres(void)
{
	const char* args[] =
	{
		"mesh",
		"_builtin_sphere",

		"mat",
		"/materials/default.nemat",

		NULL
	};

	struct ne_entity_comp_info comps[2] =
	{
		{ DRAWABLE_MESH_COMP_TYPE, args },
		{ TRANSFORM_COMP_TYPE, NULL },
	};

	float spacing = 2.5;
	for (int i = 0; i < 7; ++i) {
		float metallic = (float)i / (float)7;
		for (int j = 0; j < 7; ++j) {
			entity_handle mesh = entity_create_v(2, comps);
			struct ne_transform_comp* t = entity_get_component(mesh,
				comp_get_type_id(TRANSFORM_COMP_TYPE));
			struct ne_drawable_mesh_comp* m = entity_get_component(mesh,
				comp_get_type_id(DRAWABLE_MESH_COMP_TYPE));

			for (uint8_t k = 0; k < m->materials.count; ++k) {
				struct ne_material* mat = rt_array_get_ptr(&m->materials, k);
				mat->data.metallic = metallic;
				mat->data.roughness = kmClamp((float)j / 7.f, .05f, 1.f);
			}

			kmVec3Fill(&t->pos,
				(j - (7 / 2)) * spacing,
				(i - (7 / 2)) * spacing,
				0.f);
		}
	}
}

ne_status
tapp_init(void)
{
	comp_type_id comp[2];

	ne_status ret = NE_FAIL;

	comp[0] = comp_get_type_id(CAMERA_COMP_TYPE);
	comp[1] = comp_get_type_id(TAPP_MOVEMENT_COMPONENT_TYPE);

	ret = ecsys_register("tapp_camera_movement", ECSYS_GROUP_LOGIC,
		comp, 2, movement_system, false, 0);
	if (ret != NE_OK)
		return ret;

	rt_array_init_ptr(&widgets, 50);

	comp[0] = comp_get_type_id(TAPP_FPS_COMPONENT_TYPE);

	ret = ecsys_register("fps_sys", ECSYS_GROUP_LOGIC, comp, 1, fps_system, false, 0);
	if (ret != NE_OK)
		return ret;

	comp_create(TAPP_FPS_COMPONENT_TYPE, 0, NULL);

	comp[0] = comp_get_type_id(TAPP_DBG_COMPONENT_TYPE);

	ret = ecsys_register("dbg_sys", ECSYS_GROUP_LOGIC, comp, 1, dbg_system, false, 0);
	if (ret != NE_OK)
		return ret;

	comp_create(TAPP_DBG_COMPONENT_TYPE, 0, NULL);

	float start[4] = { .7f, .7f, .7f, 1.f };
	float end[4] = { 0.f, 0.f, 0.f, 1.f };

	h_w = gui_widget_create(10, 210, 100, 100);
	gui_widget_set_bg_gradient(h_w, start, end, GRADIENT_HORIZONTAL);

	j_w = gui_widget_create(110, 310, 100, 100);
	gui_widget_set_bg_gradient(j_w, start, end, GRADIENT_HORIZONTAL);

	k_w = gui_widget_create(110, 110, 100, 100);
	gui_widget_set_bg_gradient(k_w, start, end, GRADIENT_HORIZONTAL);

	l_w = gui_widget_create(210, 210, 100, 100);
	gui_widget_set_bg_gradient(l_w, start, end, GRADIENT_HORIZONTAL);

	cam_init(&tapp_camera);
	ne_main_camera = &tapp_camera;

	entity_handle player = entity_create(NULL);
	entity_add_component(player, comp_get_type_id(CAMERA_COMP_TYPE),
		ne_main_camera->handle);
	entity_add_new_component(player, comp_get_type_id(TAPP_MOVEMENT_COMPONENT_TYPE),
		NULL);

	struct movement_component *mc =
		entity_get_component(player, comp_get_type_id(TAPP_MOVEMENT_COMPONENT_TYPE));

	mc->light = gfx_create_light();

	kmVec4Fill(&mc->light->pos, 0.f, 0.f, -15.f, 0.f);
	kmVec4Fill(&mc->light->dir, 1.f, 1.f, 0.f, 0.f);
	kmVec4Fill(&mc->light->color, 1.f, 1.f, 1.f, 100.f);
	kmVec4Fill(&mc->light->info, 20.f, 25.f, 0.f, 0.f);

	_kat();
	//_pbr_spheres();

	return NE_OK;
}

void
tapp_update(double dt)
{
	struct ne_widget *w;
	float red[4] = { .7f, 0.f, 0.f, 1.f };
	float blue[4] = { 0.f, 0.f, .7f, 1.f };
	float green[4] = { 0.f, .7f, 0.f, 1.f };
	float purple[4] = { .7f, 0.f, .7f, 1.f };
	float gray[4] = { .7f, .7f, .7f, 1.f };
	float black[4] = { 0.f, 0.f, 0.f, 1.f };

	if (input_get_key_down(NE_KEY_H)) {
		gui_widget_set_bg_gradient(h_w, purple, black, GRADIENT_HORIZONTAL);

		rt_array_foreach_ptr(w, &widgets)
			gui_widget_set_bg_gradient(w, purple, black, GRADIENT_HORIZONTAL);
	} else if (input_get_key_up(NE_KEY_H)) {
		gui_widget_set_bg_gradient(h_w, gray, black, GRADIENT_HORIZONTAL);
	}

	if (input_get_key_down(NE_KEY_J)) {
		gui_widget_set_bg_gradient(j_w, green, black, GRADIENT_HORIZONTAL);

		rt_array_foreach_ptr(w, &widgets)
			gui_widget_set_bg_gradient(w, green, black, GRADIENT_HORIZONTAL);
	} else if (input_get_key_up(NE_KEY_J)) {
		gui_widget_set_bg_gradient(j_w, gray, black, GRADIENT_HORIZONTAL);
	}

	if (input_get_key_down(NE_KEY_K)) {
		gui_widget_set_bg_gradient(k_w, blue, black, GRADIENT_HORIZONTAL);

		rt_array_foreach_ptr(w, &widgets)
			gui_widget_set_bg_gradient(w, blue, black, GRADIENT_HORIZONTAL);
	} else if (input_get_key_up(NE_KEY_K)) {
		gui_widget_set_bg_gradient(k_w, gray, black, GRADIENT_HORIZONTAL);
	}

	if (input_get_key_down(NE_KEY_L)) {
		gui_widget_set_bg_gradient(l_w, red, black, GRADIENT_HORIZONTAL);

		rt_array_foreach_ptr(w, &widgets)
			gui_widget_set_bg_gradient(w, red, black, GRADIENT_HORIZONTAL);
	} else if (input_get_key_up(NE_KEY_L)) {
		gui_widget_set_bg_gradient(l_w, gray, black, GRADIENT_HORIZONTAL);
	}
}

void
tapp_draw(void)
{
	//
}

void
tapp_screen_resized(uint16_t width,
	uint16_t height)
{
	(void)width;
	(void)height;
}

void
tapp_destroy(void)
{
	for (size_t i = 0; i < widgets.count; ++i)
		gui_widget_destroy(rt_array_get_ptr(&widgets, i));
	rt_array_release(&widgets);

	gui_widget_destroy(h_w);
	gui_widget_destroy(j_w);
	gui_widget_destroy(k_w);
	gui_widget_destroy(l_w);
}

