/* NekoEngine
 *
 * graphics.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Graphics Subsystem
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

#include <stddef.h>

#include <system/log.h>
#include <system/config.h>
#include <system/system.h>

#include <engine/engine.h>
#include <engine/resource.h>
#include <graphics/mesh.h>
#include <graphics/texture.h>
#include <graphics/graphics.h>
#include <graphics/material.h>
#include <graphics/primitive.h>

#define GFX_MODULE	"Graphics"

uint16_t ne_gfx_screen_width;
uint16_t ne_gfx_screen_height;
const struct ne_texture *ne_blank_texture = NULL,
		*ne_blank_normal_texture = NULL;
extern struct ne_gfx_module nullgfx_module;
const struct ne_gfx_module *gfx_module = NULL;
static void *_gfx_module_handle;
static create_gfx_module_proc _gfx_create_proc;

static uint8_t _blank_tex_data[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t _blank_normal_tex_data[4] = { 0x7F, 0x7F, 0xFF, 0xFF };

ne_status
gfx_init(void)
{
	struct ne_texture_create_info tex_ci;
	const char *module = NULL;
	ne_status ret = NE_FAIL;

	memset(&tex_ci, 0x0, sizeof(tex_ci));

	module = sys_config_get_string("gfx_module", NULL);
	if (!module) {
		if (!ne_headless)
			log_entry(GFX_MODULE, LOG_WARNING,
				"No graphics module specified, falling back to null graphics");
		gfx_module = &nullgfx_module;
		goto reg_handlers;
	}

	ne_gfx_screen_width = sys_config_get_int("width", 1280.f);
	ne_gfx_screen_height = sys_config_get_int("height", 720.f);

	_gfx_module_handle = sys_load_library(module);
	if (!_gfx_module_handle) {
		log_entry(GFX_MODULE, LOG_CRITICAL,
			"Failed to load graphics module");
		ret = NE_LIBRARY_LOAD_FAIL;
		goto error;
	}

	_gfx_create_proc = (create_gfx_module_proc)
		sys_get_proc_address(_gfx_module_handle, "create_gfx_module");
	if (!_gfx_create_proc) {
		log_entry(GFX_MODULE, LOG_CRITICAL,
			"Invalid graphics module");
		ret = NE_INVALID_GFX_LIB;
		goto error;
	}

	gfx_module = _gfx_create_proc();
	if (!gfx_module) {
		log_entry(GFX_MODULE, LOG_CRITICAL,
			"Graphics module create failed");
		ret = NE_FAIL;
		goto error;
	}

	if (gfx_module->api_ver != NE_GFX_API_VER) {
		log_entry(GFX_MODULE, LOG_CRITICAL,
			"Graphics module version mismatch");
		ret = NE_API_VERSION_MISMATCH;
		goto error;
	}

	if ((ret = gfx_module->init()) != NE_OK) {
		log_entry(GFX_MODULE, LOG_CRITICAL,
			"Graphics module init failed");
		goto error;
	}

	tex_ci.type = NE_TEXTURE_2D;
	tex_ci.format = NE_IMAGE_FORMAT_R8G8B8A8_UNORM;
	tex_ci.width = 1;
	tex_ci.height = 1;
	tex_ci.depth = 1;
	tex_ci.levels = 1;
	tex_ci.layers = 1;

	ne_blank_texture = gfx_create_texture(&tex_ci,
			_blank_tex_data, sizeof(_blank_tex_data));
	if (!ne_blank_texture)
		goto error;

	ne_blank_normal_texture = gfx_create_texture(&tex_ci,
			_blank_normal_tex_data, sizeof(_blank_normal_tex_data));
	if (!ne_blank_normal_texture)
		goto error;

reg_handlers:
	if (res_register_type(RES_TEXTURE, load_texture,
		(res_unload_proc)gfx_destroy_texture) != NE_OK) {
		log_entry(GFX_MODULE, LOG_CRITICAL,
			"Failed to register texture resource handler");
		return NE_FAIL;
	}

	if (res_register_type(RES_MESH, load_mesh,
		destroy_mesh) != NE_OK) {
		log_entry(GFX_MODULE, LOG_CRITICAL,
			"Failed to register mesh resource handler");
		return NE_FAIL;
	}

	if (res_register_type(RES_MATERIAL, load_material,
		destroy_material) != NE_OK) {
		log_entry(GFX_MODULE, LOG_CRITICAL,
			"Failed to register material resource handler");
		return NE_FAIL;
	}

	if (gfx_init_primitive() != NE_OK) {
		log_entry(GFX_MODULE, LOG_CRITICAL, "Failed create primitives");
		return NE_FAIL;
	}

	return NE_OK;

error:
	gfx_module = NULL;
	_gfx_create_proc = NULL;

	if (_gfx_module_handle)
		sys_unload_library(_gfx_module_handle);
	_gfx_module_handle = NULL;

	return ret;
}

void
gfx_release(void)
{
	if (!gfx_module)
		return;

	gfx_release_primitive();

	if (ne_blank_texture)
		gfx_destroy_texture(ne_blank_texture);

	if (ne_blank_normal_texture)
		gfx_destroy_texture(ne_blank_normal_texture);

	gfx_module->release();

	gfx_module = NULL;
	_gfx_create_proc = NULL;
	sys_unload_library(_gfx_module_handle);
	_gfx_module_handle = NULL;
}
