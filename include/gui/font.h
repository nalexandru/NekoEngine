/* NekoEngine
 *
 * font.h
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

#ifndef _NE_GUI_FONT_H_
#define _NE_GUI_FONT_H_

#include <gui/guidefs.h>
#include <engine/math.h>
#include <engine/status.h>
#include <ecs/component.h>

#define FONT_NUM_CHARS		128

struct ne_font_glyph
{
	int32_t size[2];
	int32_t bearing[2];
	uint32_t adv;
	float offset;
};

struct ne_font
{
	struct ne_texture *texture;
	struct ne_font_glyph glyphs[FONT_NUM_CHARS];
	struct ne_gui_vertex *vertices;
	uint16_t *indices;
	uint32_t vtx_buff_size;
	uint32_t idx_buff_size;
	uint32_t max_chars;
	uint32_t vtx_count;
	uint32_t idx_count;
	comp_handle handle;
	uint32_t tex_width;
	uint32_t tex_height;
};

void gui_draw_text(const char *text, float x, float y, struct ne_font *font);

#ifdef _NE_ENGINE_INTERNAL_

struct ne_font_comp
{
	NE_COMPONENT;

	struct ne_font *font;
};

#define NE_FONT_COMP	"ne_font_comp"

NE_REGISTER_COMPONENT(NE_FONT_COMP, struct ne_font_comp, NULL, NULL)

#endif

#endif /* _NE_GUI_FONT_H_ */
