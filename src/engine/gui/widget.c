/* NekoEngine
 *
 * widget.c
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

#include <string.h>
#include <stdlib.h>

#include <system/log.h>

#include <gui/gui.h>
#include <gui/widget.h>
#include <gui/gui_internal.h>
#include <ecs/component.h>
#include <engine/components.h>
#include <graphics/graphics.h>

#define WIDGET_VTX_BOTTOM_LEFT		0
#define WIDGET_VTX_TOP_LEFT		1
#define WIDGET_VTX_TOP_RIGHT		2
#define WIDGET_VTX_BOTTOM_RIGHT		3

rt_array gui_widgets;

void
gui_widget_calc_rect(ne_widget *w)
{
	float left = CALC_COORDS(w->x, (float)ne_gfx_screen_width);
	float right = CALC_COORDS(w->x + w->width, (float)ne_gfx_screen_width);
	float top = CALC_COORDS(w->y, (float)ne_gfx_screen_height);
	float bottom = CALC_COORDS(w->y + w->height, (float)ne_gfx_screen_height);

	struct ne_gui_drawable_comp *comp = comp_ptr(w->comp);

	comp->vertices[WIDGET_VTX_BOTTOM_LEFT].pos_uv.x = left;
	comp->vertices[WIDGET_VTX_BOTTOM_LEFT].pos_uv.y = bottom;
	comp->vertices[WIDGET_VTX_BOTTOM_LEFT].pos_uv.z = 0.f;
	comp->vertices[WIDGET_VTX_BOTTOM_LEFT].pos_uv.w = 0.f;

	comp->vertices[WIDGET_VTX_TOP_LEFT].pos_uv.x = left;
	comp->vertices[WIDGET_VTX_TOP_LEFT].pos_uv.y = top;
	comp->vertices[WIDGET_VTX_TOP_LEFT].pos_uv.z = 0.f;
	comp->vertices[WIDGET_VTX_TOP_LEFT].pos_uv.w = 1.f;

	comp->vertices[WIDGET_VTX_TOP_RIGHT].pos_uv.x = right;
	comp->vertices[WIDGET_VTX_TOP_RIGHT].pos_uv.y = top;
	comp->vertices[WIDGET_VTX_TOP_RIGHT].pos_uv.z = 1.f;
	comp->vertices[WIDGET_VTX_TOP_RIGHT].pos_uv.w = 1.f;

	comp->vertices[WIDGET_VTX_BOTTOM_RIGHT].pos_uv.x = right;
	comp->vertices[WIDGET_VTX_BOTTOM_RIGHT].pos_uv.y = bottom;
	comp->vertices[WIDGET_VTX_BOTTOM_RIGHT].pos_uv.z = 1.f;
	comp->vertices[WIDGET_VTX_BOTTOM_RIGHT].pos_uv.w = 0.f;

	comp->dirty = true;
}
#include <engine/resource.h>
ne_widget *
gui_widget_create(
	uint16_t x,
	uint16_t y,
	uint16_t width,
	uint16_t height)
{
	ne_widget *w = NULL;

	w = calloc(1, sizeof(*w));
	if (!w)
		return NULL;

	w->comp = comp_create(GUI_DRAWABLE_COMP_TYPE, NE_INVALID_ENTITY, NULL);

	w->x = x;
	w->y = y;
	w->width = width;
	w->height = height;

	gui_widget_calc_rect(w);

	struct ne_gui_drawable_comp *comp = comp_ptr(w->comp);

	// FIXME
	comp->texture = (struct ne_texture *)res_load("/textures/anna", RES_TEXTURE);

	rt_array_add_ptr(&gui_widgets, w);

	return w;
}

void
gui_widget_set_position(
	ne_widget *w,
	uint16_t x,
	uint16_t y)
{
	w->x = x;
	w->y = y;

	gui_widget_calc_rect(w);
}

void
gui_widget_set_size(
	ne_widget *w,
	uint16_t width,
	uint16_t height)
{
	w->width = width;
	w->height = height;

	gui_widget_calc_rect(w);
}

void
gui_widget_set_enabled(
	ne_widget *w,
	bool enable)
{
	w->enabled = enable;
}

void
gui_widget_set_visibile(
	ne_widget *w,
	bool visible)
{
	struct ne_gui_drawable_comp *comp = comp_ptr(w->comp);
	comp->visible = visible;
}

void
gui_widget_set_handler(
	ne_widget *w,
	ne_gui_evt event,
	gui_handler handler)
{
	w->handlers[event] = handler;
}

void
gui_widget_set_bg_color(
	ne_widget *w,
	float color[4])
{
	struct ne_gui_drawable_comp *comp = comp_ptr(w->comp);
	memcpy(&comp->vertices[0].color, color, sizeof(comp->vertices[0].color));
	memcpy(&comp->vertices[1].color, color, sizeof(comp->vertices[1].color));
	memcpy(&comp->vertices[2].color, color, sizeof(comp->vertices[2].color));
	memcpy(&comp->vertices[3].color, color, sizeof(comp->vertices[3].color));
	comp->dirty = true;
}

void
gui_widget_set_bg_gradient(
	ne_widget *w,
	float start[4],
	float end[4],
	ne_gradient_type type)
{
	struct ne_gui_drawable_comp *comp = comp_ptr(w->comp);

	if (type == GRADIENT_HORIZONTAL) {
		memcpy(&comp->vertices[WIDGET_VTX_TOP_LEFT].color.x, start,
			sizeof(comp->vertices[WIDGET_VTX_TOP_LEFT].color));
		memcpy(&comp->vertices[WIDGET_VTX_TOP_RIGHT].color.x, start,
			sizeof(comp->vertices[WIDGET_VTX_TOP_RIGHT].color));
		memcpy(&comp->vertices[WIDGET_VTX_BOTTOM_LEFT].color.x, end,
			sizeof(comp->vertices[WIDGET_VTX_BOTTOM_LEFT].color));
		memcpy(&comp->vertices[WIDGET_VTX_BOTTOM_RIGHT].color.x, end,
			sizeof(comp->vertices[WIDGET_VTX_BOTTOM_RIGHT].color));
	} else if (type == GRADIENT_VERTICAL) {
		memcpy(&comp->vertices[WIDGET_VTX_TOP_LEFT].color.x, start,
			sizeof(comp->vertices[WIDGET_VTX_TOP_LEFT].color));
		memcpy(&comp->vertices[WIDGET_VTX_BOTTOM_LEFT].color.x, start,
			sizeof(comp->vertices[WIDGET_VTX_BOTTOM_LEFT].color));
		memcpy(&comp->vertices[WIDGET_VTX_TOP_RIGHT].color.x, end,
			sizeof(comp->vertices[WIDGET_VTX_TOP_RIGHT].color));
		memcpy(&comp->vertices[WIDGET_VTX_BOTTOM_RIGHT].color.x, end,
			sizeof(comp->vertices[WIDGET_VTX_BOTTOM_RIGHT].color));
	}

	comp->dirty = true;
}

void
gui_widget_destroy(ne_widget *w)
{
	comp_destroy(w->comp);
	free(w);
}
