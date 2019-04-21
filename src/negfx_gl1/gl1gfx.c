/* NekoEngine
 *
 * gl1gfx.c
 * Author: Alexandru Naiman
 *
 * NekoEngine OpenGL 1 Graphics Subsystem
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
#include <system/log.h>
#include <graphics/mesh.h>
#include <system/config.h>
#include <engine/resource.h>
#include <graphics/texture.h>
#include <graphics/graphics.h>

#include <gui.h>
#include <gl1gfx.h>
#include <buffer.h>
#include <texture.h>

#define GL_GFX_MODULE	"OpenGL1_Graphics"

uint16_t _width = 0,
	_height = 0;
enum gl1gfx_view_matrix_type gl1gfx_view_matrix = MAT_NONE;

ne_status gl1gfx_init(void);
void gl1gfx_draw(void);
void gl1gfx_release(void);
void gl1gfx_screen_resized(uint16_t, uint16_t);
void gl1gfx_wait_idle(void) { }

ne_status gl1gfx_init_material(struct ne_material *mat) { return NE_OK; }
void gl1gfx_release_material(struct ne_material *mat) { }

struct ne_gfx_module gl1gfx_module =
{
	NE_GFX_API_VER,
	gl1gfx_init,
	gl1gfx_draw,
	gl1gfx_screen_resized,
	NULL, //gfx_sys_swap_interval,
	gl1gfx_wait_idle,
	gl1gfx_init_gui_drawable,
	gl1gfx_free_gui_drawable,

	gl1gfx_create_texture,
	gl1gfx_upload_image,
	gl1gfx_destroy_texture,

	gl1gfx_register_font,
	gl1gfx_unregister_font,

	gl1gfx_create_buffer,
	gl1gfx_map_buffer,
	gl1gfx_unmap_buffer,
	gl1gfx_copy_buffer,
	gl1gfx_upload_buffer,
	gl1gfx_flush_buffer,
	gl1gfx_invalidate_buffer,
	gl1gfx_destroy_buffer,

	gl1gfx_init_material,
	gl1gfx_release_material,

	gl1gfx_init_drawable_mesh,
	gl1gfx_release_drawable_mesh,
	gl1gfx_add_mesh,

	gl1gfx_create_light,
	gl1gfx_destroy_light,

	gl1gfx_release,

	false,
	0	// Set after context creation
};

const struct ne_gfx_module *
create_gfx_module(void)
{
	return &gl1gfx_module;
}

ne_status
gl1gfx_init(void)
{
	GLint ver_major = 1, ver_minor = 5;
	ne_status ret = NE_FAIL;

	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Initializing...");

	_width = (uint16_t)sys_config_get_int("width", 0);
	_height = (uint16_t)sys_config_get_int("height", 0);

	if ((ret = gfx_sys_init(false)) != NE_OK)
		return ret;

	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Context: OpenGL %d.%d",
		ver_major, ver_minor);
	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Vendor: %s",
		glGetString(GL_VENDOR));
	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Renderer: %s",
		glGetString(GL_RENDERER));
	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Version: %s",
		glGetString(GL_VERSION));

	/*if (res_register_type(RES_TEXTURE, texture_load_from_file,
		texture_destroy) != NE_OK) {
		log_entry(GL_GFX_MODULE, LOG_CRITICAL,
			"Failed to register texture resource handler");
		return NE_FAIL;
	}*/

	glClearColor(.8f, 0.f, .6f, 1.f);

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_LIGHT0);

	/*GLfloat ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse[] = { 10.0f, 10.0f, 10.0f, 1.0f };
	GLfloat position[] = { 0.5f, 1.0f, 0.0f, 0.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);*/

	gl1gfx_init_render();

	return gl1gfx_init_gui();
}

void
gl1gfx_draw(void)
{
	glViewport(0, 0, _width, _height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	ecsys_update_group(ECSYS_GROUP_RENDER);

	gl1gfx_render_scene();

	gl1gfx_draw_gui();

	gfx_sys_swap_buffers();
}

void
gl1gfx_screen_resized(
	uint16_t width,
	uint16_t height)
{
	_width = width;
	_height = height;

	glViewport(0, 0, _width, _height);
}

void
gl1gfx_release(void)
{
	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Shutting down...");

	gl1gfx_release_render();

	gfx_sys_destroy();

	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Shut down complete");
}

