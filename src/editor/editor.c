/* NekoEditor
 *
 * editor.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Editor
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

#include <ecs/ecsys.h>
#include <ecs/entity.h>
#include <scene/camera.h>
#include <engine/resource.h>
#include <engine/components.h>
#include <engine/application.h>

#include <editor/ui.h>
#include <editor/editor.h>
#include <editor/project.h>

ne_status editor_init(void);
void editor_update(double);
void editor_draw(void);
void editor_screen_resized(uint16_t, uint16_t);
void editor_destroy(void);

ne_app_module _editor_app_module =
{
	NE_APP_API_VER,
	editor_init,
	editor_update,
	editor_draw,
	editor_screen_resized,
	editor_destroy
};

struct ne_font *_editor_font;
struct ne_camera _editor_camera;

const ne_app_module *
create_app_module(void)
{
	return &_editor_app_module;
}

ne_status
editor_init(void)
{
	/*comp_type_id comp[2];
	ne_status ret = NE_FAIL;

	comp[0] = comp_get_type_id(CAMERA_COMP_TYPE);
	comp[1] = comp_get_type_id(EDITOR_CAMERA_MOVEMENT_COMP_TYPE);

	ret = ecsys_register("editor_camera_movement", ECSYS_GROUP_LOGIC,
		comp, 2, editor_camera_movement_sys, false, 0);
	if (ret != NE_OK)
		return ret;

	_editor_font = res_load("/fonts/plex.ttf", RES_FONT);

	cam_init(&_editor_camera);
	ne_main_camera = &_editor_camera;

	entity_handle player = entity_create(NULL);
	entity_add_component(player, comp_get_type_id(CAMERA_COMP_TYPE),
		ne_main_camera->handle);
	entity_add_new_component(player, comp_get_type_id(EDITOR_CAMERA_MOVEMENT_COMP_TYPE),
		NULL);*/

	const char *proj = edui_open_proj_dlg();

	if (!proj)
		return NE_ABORT_START;

	if (load_proj(proj) != NE_OK)
		return NE_ABORT_START;

	return NE_OK;
}

void
editor_update(double dt)
{
}

void
editor_draw(void)
{
}

void
editor_screen_resized(
	uint16_t width,
	uint16_t height)
{
	(void)width;
	(void)height;
}

void
editor_destroy(void)
{
	res_unload(_editor_font, RES_FONT);
}

