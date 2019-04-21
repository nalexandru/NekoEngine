/* NekoEngine
 *
 * d3d12gfx.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Direct3D 12 Graphics Subsystem
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
#include <d3d12gfx.h>
#include <buffer.h>
#include <render.h>
#include <texture.h>

#define D3D12GFX_MODULE	"Direct3D12_Graphics"

ne_status d3d12gfx_init(void);
void d3d12gfx_draw(void);
void d3d12gfx_release(void);
void d3d12gfx_screen_resized(uint16_t, uint16_t);
void d3d12gfx_wait_idle(void) { }



struct ne_gfx_module _d3d12_gfx_module =
{
	NE_GFX_API_VER,
	d3d12gfx_init,
	d3d12gfx_draw,
	d3d12gfx_screen_resized,
	NULL, //gfx_sys_swap_interval,
	d3d12gfx_wait_idle,
	d3d12gfx_init_gui_drawable,
	d3d12gfx_free_gui_drawable,

	d3d12gfx_create_texture,
	d3d12gfx_upload_image,
	d3d12gfx_destroy_texture,

	d3d12gfx_register_font,
	d3d12gfx_unregister_font,

	d3d12gfx_create_buffer,
	d3d12gfx_map_buffer,
	d3d12gfx_unmap_buffer,
	d3d12gfx_copy_buffer,
	d3d12gfx_upload_buffer,
	d3d12gfx_flush_buffer,
	d3d12gfx_invalidate_buffer,
	d3d12gfx_destroy_buffer,

	d3d12gfx_init_material,
	d3d12gfx_release_material,

	d3d12gfx_init_drawable_mesh,
	d3d12gfx_release_drawable_mesh,
	d3d12gfx_add_mesh,

	d3d12gfx_release,

	true
};

const struct ne_gfx_module *
create_gfx_module(void)
{
	return &_d3d12_gfx_module;
}

ne_status
d3d12gfx_init(void)
{
	return NE_OK;
}

void
d3d12gfx_draw(void)
{
}

void
d3d12gfx_screen_resized(uint16_t width,
	uint16_t height)
{

}

void
d3d12gfx_release(void)
{
	log_entry(D3D12GFX_MODULE, LOG_INFORMATION, "Shutting down...");

	log_entry(D3D12GFX_MODULE, LOG_INFORMATION, "Shut down complete");
}

