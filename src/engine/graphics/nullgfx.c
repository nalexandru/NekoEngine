/* NekoEngine
 *
 * null.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Null Graphics Subsystem
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

#include <gui/font.h>
#include <graphics/graphics.h>

struct ne_buffer
{
	void *map;
};

ne_status nullgfx_init(void);
void nullgfx_draw(void) { }
void nullgfx_screen_resized(uint16_t width, uint16_t height) { }
void nullgfx_swap_interval(int swi) { }
void nullgfx_wait_idle(void) { }
ne_status nullgfx_init_gui_drawable(struct ne_gui_drawable_comp *draw) { return NE_OK; }
void nullgfx_free_gui_drawable(struct ne_gui_drawable_comp *draw) { }
struct ne_texture *nullgfx_create_texture(struct ne_texture_create_info *, const void *, uint64_t);
ne_status nullgfx_upload_image(struct ne_texture *t, const void *d, uint64_t s) { }
void nullgfx_destroy_texture(struct ne_texture *t);
ne_status nullgfx_register_font(struct ne_font *);
void nullgfx_unregister_font(struct ne_font *);
struct ne_buffer *nullgfx_create_buffer(struct ne_buffer_create_info *, const void *, uint64_t, uint64_t);
ne_status nullgfx_map_buffer(struct ne_buffer *, uint64_t, uint64_t, void **);
void nullgfx_unmap_buffer(struct ne_buffer *buff);
ne_status nullgfx_upload_buffer(struct ne_buffer *buff, const void *data, uint64_t offset, uint64_t size) { return NE_OK; }
ne_status nullgfx_copy_buffer(struct ne_buffer *dst, struct ne_buffer *src, uint64_t size, uint64_t dst_offset, uint64_t src_offset) { return NE_OK; }
ne_status nullgfx_flush_buffer(struct ne_buffer *buff) { return NE_OK; }
ne_status nullgfx_invalidate_buffer(struct ne_buffer *buff) { return NE_OK; }
void nullgfx_destroy_buffer(struct ne_buffer *buff);
ne_status nullgfx_init_material(struct ne_material *mat) { return NE_OK; }
void nullgfx_release_material(struct ne_material *mat) { }
ne_status nullgfx_init_drawable_mesh(struct ne_drawable_mesh_comp *comp) { return NE_OK; }
void nullgfx_release_drawable_mesh(struct ne_drawable_mesh_comp *comp) { }
void nullgfx_add_mesh(struct ne_drawable_mesh_comp *comp, size_t group) { }
void nullgfx_release(void) { }
void *nullgfx_create_res(const char *);
void nullgfx_destroy_res(void *);

struct ne_gfx_module nullgfx_module =
{
	NE_GFX_API_VER,
	nullgfx_init,
	nullgfx_draw,
	nullgfx_screen_resized,
	nullgfx_swap_interval,
	nullgfx_wait_idle,
	nullgfx_init_gui_drawable,
	nullgfx_free_gui_drawable,
	nullgfx_create_texture,
	nullgfx_upload_image,
	nullgfx_destroy_texture,
	nullgfx_register_font,
	nullgfx_unregister_font,
	nullgfx_create_buffer,
	nullgfx_map_buffer,
	nullgfx_unmap_buffer,
	nullgfx_copy_buffer,
	nullgfx_upload_buffer,
	nullgfx_flush_buffer,
	nullgfx_invalidate_buffer,
	nullgfx_destroy_buffer,
	nullgfx_init_material,
	nullgfx_release_material,
	nullgfx_init_drawable_mesh,
	nullgfx_release_drawable_mesh,
	nullgfx_add_mesh,
	nullgfx_release,
	false
};

ne_status
nullgfx_init()
{
	return NE_OK;
}

ne_status
nullgfx_register_font(struct ne_font *font)
{
	font->vertices = calloc(1, font->vtx_buff_size);
	font->indices = calloc(1, font->idx_buff_size);

	return NE_OK;
}

void
nullgfx_unregister_font(struct ne_font *font)
{
	free(font->vertices);
	free(font->indices);
}

struct ne_texture *
nullgfx_create_texture(
	struct ne_texture_create_info *ci,
	const void *d,
	uint64_t s)
{
	return malloc(sizeof(uint32_t));
}

void
nullgfx_destroy_texture(struct ne_texture *t)
{
	free(t);
}

struct ne_buffer *
nullgfx_create_buffer(
	struct ne_buffer_create_info *ci,
	const void *data,
	uint64_t offset,
	uint64_t size)
{
	return calloc(1, sizeof(struct ne_buffer));
}

ne_status
nullgfx_map_buffer(
	struct ne_buffer *buff,
	uint64_t offset,
	uint64_t size,
	void **ptr)
{
	buff->map = malloc(size);
	*ptr = buff->map;
	return *ptr == NULL ? NE_FAIL : NE_OK;
}

void
nullgfx_unmap_buffer(struct ne_buffer *buff)
{
	free(buff->map);
	buff->map = NULL;
}

void
nullgfx_destroy_buffer(struct ne_buffer *buff)
{
	free(buff->map);
	free(buff);
}

void *
nullgfx_create_res(const char *c)
{
	return malloc(sizeof(uint32_t));
}

void
nullgfx_destroy_res(void *res)
{
	free(res);
}
