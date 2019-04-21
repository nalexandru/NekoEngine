/* NekoEngine
 *
 * gui.c
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

#include <gui/font.h>
#include <ecs/ecsys.h>
#include <engine/math.h>
#include <engine/components.h>
#include <runtime/runtime.h>

#include <gui.h>
#include <gl1gfx.h>
#include <texture.h>

struct gui_draw_info
{
	uint32_t vtx_count;
	uint32_t idx_count;
};

struct gui_font_info
{
	struct ne_font *font;
};

static rt_array _gui_fonts;
static uint8_t _gui_indices[6] = { 0, 1, 2, 0, 2, 3 };

void gui_draw_system(double dt, void **comp);

ne_status
gl1gfx_init_gui(void)
{
	ne_status ret;
	comp_type_id comp_type;

	comp_type = comp_get_type_id(GUI_DRAWABLE_COMP_TYPE);

	ret = ecsys_register("gl1_gui_draw", ECSYS_GROUP_RENDER,
			&comp_type, 1, gui_draw_system, false, 1000);

	if (ret != NE_OK)
		return ret;

	rt_array_init(&_gui_fonts, 10, sizeof(_gui_fonts));

	return NE_OK;
}

ne_status
gl1gfx_register_font(struct ne_font *font)
{
	struct gui_font_info info;
	memset(&info, 0x0, sizeof(info));

	info.font = font;
	font->vertices = calloc(1, font->vtx_buff_size);
	font->indices = calloc(1, font->idx_buff_size);

	rt_array_add(&_gui_fonts, &info);

	return NE_OK;
}

void
gl1gfx_unregister_font(struct ne_font *font)
{
	free(font->vertices);
	free(font->indices);
}

ne_status
gl1gfx_init_gui_drawable(struct ne_gui_drawable_comp *comp)
{
	return NE_OK;
}

void
gl1gfx_free_gui_drawable(struct ne_gui_drawable_comp *comp)
{
}

void
gl1gfx_draw_gui(void)
{
	if (gl1gfx_view_matrix != MAT_GUI) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

//		glOrtho(0.f, (float)sys_config_get_int("width", 0),
//				(float)sys_config_get_int("height", 0), 0.f, 0.f, 1.f);
		gl1gfx_view_matrix = MAT_GUI;
	}

	for (size_t i = 0; i < _gui_fonts.count; ++i) {
		struct gui_font_info *info = rt_array_get(&_gui_fonts, i);

		if (!info->font->idx_count)
			continue;

		glBindTexture(info->font->texture->target, info->font->texture->id);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBegin(GL_TRIANGLES);
		{
			// 0, 1, 2, 0, 2, 3
			for (uint32_t i = 0; i < info->font->idx_count; ++i) {
				uint16_t idx = info->font->indices[i];

				glColor4f(info->font->vertices[idx].color.x,
						  info->font->vertices[idx].color.y,
						  info->font->vertices[idx].color.z,
						  info->font->vertices[idx].color.w);
				glTexCoord2f(info->font->vertices[idx].pos_uv.z,
							 info->font->vertices[idx].pos_uv.w);
				glVertex2f(info->font->vertices[idx].pos_uv.x,
						   info->font->vertices[idx].pos_uv.y * -1);
			}
		}
		glEnd();

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glDisable(GL_BLEND);

		glBindTexture(info->font->texture->target, 0);
	}
}

void
gui_draw_system(
	double dt,
	void **comp)
{
	struct ne_gui_drawable_comp *draw = comp[0];
	struct gui_draw_info *info = (struct gui_draw_info *)&draw->private;

	if (gl1gfx_view_matrix != MAT_GUI) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

//		glOrtho(0.f, (float)sys_config_get_int("width", 0),
//				(float)sys_config_get_int("height", 0), 0.f, 0.f, 1.f);
		gl1gfx_view_matrix = MAT_GUI;
	}

	glBindTexture(draw->texture->target, draw->texture->id);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);

	glBegin(GL_TRIANGLES);
	{
		// 0, 1, 2, 0, 2, 3
		for (uint8_t i = 0; i < 6; ++i) {
			uint8_t idx = _gui_indices[i];
			glColor4f(draw->vertices[idx].color.x,
					draw->vertices[idx].color.y,
					draw->vertices[idx].color.z,
					draw->vertices[idx].color.w);
			glTexCoord2f(draw->vertices[idx].pos_uv.z,
						 draw->vertices[idx].pos_uv.w);
			glVertex2f(draw->vertices[idx].pos_uv.x,
					draw->vertices[idx].pos_uv.y * -1);
		}
	}
	glEnd();

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBindTexture(draw->texture->target, 0);

//	glFlush();

/*	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, info->vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);*/
}

void
gl1gfx_release_gui(void)
{
	rt_array_release(&_gui_fonts);
}

