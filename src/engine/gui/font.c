/* NekoEngine
 *
 * font.c
 * Author: Alexandru Naiman
 *
 * NekoEngine GUI Subsystem
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

#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <system/log.h>

#include <gui/font.h>
#include <gui/gui_internal.h>
#include <engine/io.h>
#include <graphics/vertex.h>
#include <graphics/graphics.h>

#define FONT_START_CHAR		32
#define FONT_MODULE		"Font"

static inline bool
_create_buffer(struct ne_font *font)
{
	font->max_chars = ((1.f / (((float)font->tex_width / (float)FONT_NUM_CHARS) *
			(1.f / ne_gfx_screen_width))) *
			(1.f / (font->tex_height * (1.f / ne_gfx_screen_width))));

	// one quad for each character
	font->vtx_buff_size = font->max_chars * 4 * sizeof(struct ne_gui_vertex);
	font->idx_buff_size = font->max_chars * 6 * sizeof(uint16_t);

	if (gfx_register_font(font) != NE_OK) {
		log_entry(FONT_MODULE, LOG_CRITICAL, "Failed to register font");
		return false;
	}

	return true;
}

void *
gui_load_font(const char *path)
{
	struct ne_font *font = NULL;
	struct ne_texture_create_info tex_ci;
	FT_Face face = NULL;
	FT_GlyphSlot glyph = NULL;
	FT_Library ft_lib = NULL;
	ne_file *file = NULL;
	void *data = NULL;
	int64_t data_size = 0;
	uint8_t *tex_data = NULL;
	uint64_t tex_size = 0;
	uint32_t x = 0;

	font = calloc(1, sizeof(*font));
	if (!font)
		return NULL;

	if (FT_Init_FreeType(&ft_lib)) {
		goto error;
	}

	file = io_open(path, IO_READ);
	if (!file) {
		log_entry(FONT_MODULE, LOG_CRITICAL, "Failed to open font face");
		goto error;
	}

	data = io_read_blob(file, &data_size);
	if (!data) {
		log_entry(FONT_MODULE, LOG_CRITICAL, "Failed to read font face");
		goto error;
	}

	if (FT_New_Memory_Face(ft_lib, data, (FT_Long)data_size, 0, &face)) {
		log_entry(FONT_MODULE, LOG_CRITICAL, "Failed to load font face");
		goto error;
	}

	FT_Set_Pixel_Sizes(face, 0, 20);

	glyph = face->glyph;

	for (int i = 0; i < FONT_NUM_CHARS; ++i) {
		//FT_UInt glyph_id = FT_Get_Char_Index(face, i);
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
			continue;

		font->tex_width += glyph->bitmap.width;
		font->tex_height = font->tex_height > glyph->bitmap.rows ?
					font->tex_height : glyph->bitmap.rows;
	}

	tex_size = font->tex_width * font->tex_height;
	tex_data = calloc(sizeof(uint8_t), tex_size);
	if (!tex_data) {
		log_entry(FONT_MODULE, LOG_CRITICAL,
			"Failed to allocate texture data memory");
		goto error;
	}

	for (int i = 0; i < FONT_NUM_CHARS; ++i) {
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
			continue;

		for (uint32_t j = 0; j < glyph->bitmap.rows; ++j) {
			uint32_t offset = x + font->tex_width * j;
			uint32_t size = glyph->bitmap.width;
			uint32_t data_offset = size * j;

			memcpy(tex_data + offset, glyph->bitmap.buffer + data_offset, size);
		}

		font->glyphs[i].size[0] = glyph->bitmap.width;
		font->glyphs[i].size[1] = glyph->bitmap.rows;
		font->glyphs[i].bearing[0] = glyph->bitmap_left;
		font->glyphs[i].bearing[1] = glyph->bitmap_top;
		font->glyphs[i].adv = (uint32_t)glyph->advance.x >> 6;
		font->glyphs[i].offset = (float)x / font->tex_width;

		x += glyph->bitmap.width;
	}

	tex_ci.width = font->tex_width;
	tex_ci.height = font->tex_height;
	tex_ci.depth = tex_ci.layers = tex_ci.levels = 1;
	tex_ci.format = NE_IMAGE_FORMAT_R8_UNORM;
	tex_ci.type = NE_TEXTURE_2D;

	font->texture = gfx_create_texture(&tex_ci, tex_data, tex_size);
	if (!font->texture) {
		log_entry(FONT_MODULE, LOG_CRITICAL,
			"Failed to create texture memory");
		goto error;
	}

	if (!_create_buffer(font))
		goto error;

	font->handle = comp_create(NE_FONT_COMP, NULL, NULL);
	if (font->handle == NE_INVALID_COMPONENT)
		goto error;

	struct ne_font_comp *comp = comp_ptr(font->handle);
	comp->font = font;

	FT_Done_Face(face);

	FT_Done_FreeType(ft_lib);

	free(data);
	free(tex_data);

	return font;

error:
	if (ft_lib)
		FT_Done_FreeType(ft_lib);

	if (font->texture)
		gfx_destroy_texture(font->texture);

	free(font);
	free(data);
	free(tex_data);

	return NULL;
}

void
gui_draw_text(
	const char *text,
	float pos_x,
	float pos_y,
	struct ne_font *font)
{
	pos_y += font->tex_height;

	while (*text) {
		struct ne_font_glyph *glyph = &font->glyphs[*text++];

		float x = pos_x + glyph->bearing[0];
		float y = pos_y - glyph->bearing[1];
		float w = glyph->size[0];
		float h = glyph->size[1];

		font->indices[font->idx_count++] = font->vtx_count;
		font->indices[font->idx_count++] = font->vtx_count + 1;
		font->indices[font->idx_count++] = font->vtx_count + 2;
		font->indices[font->idx_count++] = font->vtx_count;
		font->indices[font->idx_count++] = font->vtx_count + 2;
		font->indices[font->idx_count++] = font->vtx_count + 3;

		struct ne_gui_vertex *v = &font->vertices[font->vtx_count++];
		v->color.x = v->color.y = v->color.z = 1.f;
		v->color.w = 0.f;
		v->pos_uv.x = CALC_COORDS(x, (float)ne_gfx_screen_width);
		v->pos_uv.y = CALC_COORDS(y, (float)ne_gfx_screen_height);
		v->pos_uv.z = glyph->offset;
		v->pos_uv.w = 0.f;

		v = &font->vertices[font->vtx_count++];
		v->color.x = v->color.y = v->color.z = 1.f;
		v->color.w = 0.f;
		v->pos_uv.x = CALC_COORDS(x, (float)ne_gfx_screen_width);
		v->pos_uv.y = CALC_COORDS(y + h, (float)ne_gfx_screen_height);
		v->pos_uv.z = glyph->offset;
		v->pos_uv.w = (float)glyph->size[1] / (float)font->tex_height;

		v = &font->vertices[font->vtx_count++];
		v->color.x = v->color.y = v->color.z = 1.f;
		v->color.w = 0.f;
		v->pos_uv.x = CALC_COORDS(x + w, (float)ne_gfx_screen_width);
		v->pos_uv.y = CALC_COORDS(y + h, (float)ne_gfx_screen_height);
		v->pos_uv.z = glyph->offset + ((float)glyph->size[0] / (float)font->tex_width);
		v->pos_uv.w = (float)glyph->size[1] / (float)font->tex_height;

		v = &font->vertices[font->vtx_count++];
		v->color.x = v->color.y = v->color.z = 1.f;
		v->color.w = 0.f;
		v->pos_uv.x = CALC_COORDS(x + w, (float)ne_gfx_screen_width);
		v->pos_uv.y = CALC_COORDS(y, (float)ne_gfx_screen_height);
		v->pos_uv.z = glyph->offset + ((float)glyph->size[0] / (float)font->tex_width);
		v->pos_uv.w = 0.f;

		pos_x += glyph->adv;
	}
}

void
gui_destroy_font(void *ptr)
{
	struct ne_font *font = ptr;

	gfx_unregister_font(font);

	comp_destroy(font->handle);

	gfx_destroy_texture(font->texture);

	free(ptr);
}

void
gui_reset_font_sys(
	double dt,
	void **comp)
{
	(void)dt;
	struct ne_font_comp *fc = comp[0];

	fc->font->vtx_count = fc->font->idx_count = 0;
	memset(fc->font->vertices, 0x0, fc->font->vtx_buff_size);
	memset(fc->font->indices, 0x0, fc->font->idx_buff_size);
}
