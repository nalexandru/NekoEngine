/* NekoEngine
 *
 * graphics.h
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

#ifndef _NE_GRAPHICS_H_
#define _NE_GRAPHICS_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <system/platform.h>

#include <engine/math.h>
#include <engine/status.h>

#include <graphics/buffer.h>
#include <graphics/texture.h>

#define NE_GFX_API_VER		9

struct ne_font;
struct ne_mesh;
struct ne_buffer;
struct ne_texture;
struct ne_material;
struct ne_gui_drawable_comp;
struct ne_drawable_mesh_comp;

struct ne_light
{
	kmVec4 pos;
	kmVec4 dir;
	kmVec4 color;
	kmVec4 info;
};

struct ne_gfx_module
{
	uint32_t api_ver;
	ne_status (*init)(void);
	void (*draw)(void);
	void (*screen_resized)(uint16_t, uint16_t);
	void (*swap_interval)(int);
	void (*wait_idle)(void);
	ne_status (*init_gui_drawable)(struct ne_gui_drawable_comp *);
	void (*release_gui_drawable)(struct ne_gui_drawable_comp *);

	// Texture
	struct ne_texture *(*create_texture)(const struct ne_texture_create_info *, const void *, uint64_t);
	ne_status (*upload_image)(const struct ne_texture *, const void *, uint64_t);
	void (*destroy_texture)(const struct ne_texture *);

	// Font
	ne_status (*register_font)(struct ne_font *);
	void (*unregister_font)(struct ne_font *);

	// Buffer
	struct ne_buffer *(*create_buffer)(struct ne_buffer_create_info *, const void *, uint64_t, uint64_t);
	ne_status (*map_buffer)(struct ne_buffer *, uint64_t, uint64_t, void **);
	void (*unmap_buffer)(struct ne_buffer *);
	ne_status (*copy_buffer)(struct ne_buffer *, struct ne_buffer *, uint64_t, uint64_t, uint64_t);
	ne_status (*upload_buffer)(struct ne_buffer *, const void *, uint64_t, uint64_t);
	ne_status (*flush_buffer)(struct ne_buffer *);
	ne_status (*invalidate_buffer)(struct ne_buffer *);
	void (*destroy_buffer)(struct ne_buffer *);

	// Material
	ne_status (*init_material)(struct ne_material *mat);
	void (*release_material)(struct ne_material *mat);

	// Mesh
	ne_status (*init_drawable_mesh)(struct ne_drawable_mesh_comp *comp);
	void (*release_drawable_mesh)(struct ne_drawable_mesh_comp *comp);

	void (*add_mesh)(struct ne_drawable_mesh_comp *comp, size_t group);

	// Light
	struct ne_light *(*create_light)(void);
	void (*destroy_light)(struct ne_light *);

	void (*release)(void);

	bool flip_view;
	uint32_t max_lights;
};

typedef const struct ne_gfx_module *(*create_gfx_module_proc)(void);
/*
 * Graphics modules must implement a C linked function
 * returning a ne_gfx_module struct called
 * create_gfx_module. The returned pointer must
 * remain valid for the application lifetime.
 *
 * const struct ne_gfx_module *create_gfx_module(void);
 *
 * On Win32 this function must be exported
 */

MIWA_EXPORT extern const struct ne_gfx_module *gfx_module;
MIWA_EXPORT extern const struct ne_texture *ne_blank_texture;
MIWA_EXPORT extern const struct ne_texture *ne_blank_normal_texture;

ne_status	gfx_init(void);
void		gfx_release(void);

static inline ne_status
gfx_init_gui_drawable(struct ne_gui_drawable_comp *comp)
{
	return gfx_module->init_gui_drawable(comp);
}

static inline void
gfx_release_gui_drawable(struct ne_gui_drawable_comp *comp)
{
	gfx_module->release_gui_drawable(comp);
}

static inline void
gfx_draw(void)
{
	gfx_module->draw();
}

static inline void
gfx_screen_resized(uint16_t width, uint16_t height)
{
#ifdef SYS_PLATFORM_WINDOWS
	if (gfx_module)
#endif
	gfx_module->screen_resized(width, height);
}

static inline void
gfx_wait_idle(void)
{
	gfx_module->wait_idle();
}

static inline struct ne_texture *
gfx_create_texture(const struct ne_texture_create_info *ci,
	const void *data,
	uint64_t size)
{
	return gfx_module->create_texture(ci, data, size);
}

static inline ne_status
gfx_upload_image(const struct ne_texture *tex,
	const void *data,
	uint64_t size)
{
	return gfx_module->upload_image(tex, data, size);
}

static inline void
gfx_destroy_texture(const struct ne_texture *tex)
{
	gfx_module->destroy_texture(tex);
}

static inline ne_status
gfx_register_font(struct ne_font *font)
{
	return gfx_module->register_font(font);
}

static inline void
gfx_unregister_font(struct ne_font *font)
{
	gfx_module->unregister_font(font);
}

static inline struct ne_buffer *
gfx_create_buffer(struct ne_buffer_create_info *ci,
	const void *data,
	uint64_t offset,
	uint64_t size)
{
	return gfx_module->create_buffer(ci, data, offset, size);
}

static inline ne_status
gfx_map_buffer(struct ne_buffer *buff,
	uint64_t offset,
	uint64_t size,
	void **dst)
{
	return gfx_module->map_buffer(buff, offset, size, dst);
}

static inline void
gfx_unmap_buffer(struct ne_buffer *buff)
{
	gfx_module->unmap_buffer(buff);
}

static inline ne_status
gfx_copy_buffer(struct ne_buffer *dst,
	struct ne_buffer *src,
	uint64_t size,
	uint64_t dst_offset,
	uint64_t src_offset)
{
	return gfx_module->copy_buffer(dst, src, size, dst_offset, src_offset);
}

static inline ne_status
gfx_upload_buffer(struct ne_buffer *buff,
	const void *data,
	uint64_t offset,
	uint64_t size)
{
	return gfx_module->upload_buffer(buff, data, offset, size);
}

static inline ne_status
gfx_flush_buffer(struct ne_buffer *buff)
{
	return gfx_module->flush_buffer(buff);
}

static inline ne_status
gfx_invalidate_buffer(struct ne_buffer *buff)
{
	return gfx_module->invalidate_buffer(buff);
}

static inline void
gfx_destroy_buffer(struct ne_buffer *buff)
{
	gfx_module->destroy_buffer(buff);
}

static inline ne_status
gfx_init_material(struct ne_material *mat)
{
	return gfx_module->init_material(mat);
}

static inline void
gfx_release_material(struct ne_material *mat)
{
	gfx_module->release_material(mat);
}

static inline ne_status
gfx_init_drawable_mesh(struct ne_drawable_mesh_comp *comp)
{
	return gfx_module->init_drawable_mesh(comp);
}

static inline  void
gfx_release_drawable_mesh(struct ne_drawable_mesh_comp *comp)
{
	gfx_module->release_drawable_mesh(comp);
}

static inline void
gfx_add_mesh(struct ne_drawable_mesh_comp *comp,
	size_t group)
{
	gfx_module->add_mesh(comp, group);
}

static inline struct ne_light *
gfx_create_light(void)
{
	return gfx_module->create_light();
}

static inline void
gfx_destroy_light(struct ne_light *l)
{
	gfx_module->destroy_light(l);
}

static inline bool
gfx_flip_view(void)
{
	return gfx_module->flip_view;
}

MIWA_EXPORT extern uint16_t ne_gfx_screen_width;
MIWA_EXPORT extern uint16_t ne_gfx_screen_height;

#endif /* _NE_GRAPHICS_H_ */
