/* NekoEngine
 *
 * gui_internal.h
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

#ifndef _NE_GUI_INTERNAL_H_
#define _NE_GUI_INTERNAL_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <runtime/runtime.h>

#include <engine/math.h>
#include <ecs/component.h>

#define CALC_COORDS(x, x_max) ((float)(x) / (float)(x_max) * 2) - 1

extern rt_array gui_widgets;

struct ne_widget
{
	gui_handler handlers[NUM_GUI_EVT];
	comp_handle comp;
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;

	bool enabled;
};

struct ne_label
{
	struct ne_widget base;
};

struct ne_slider
{
	struct ne_widget base;

	int32_t min;
	int32_t max;
	float value;
	bool dragging;
	bool vertical;
};

struct ne_button
{
	struct ne_widget base;
};

struct ne_text_box
{
	struct ne_widget base;

	bool shift;
};

void  gui_reset_font_sys(double, void **);

void *gui_load_font(const char *);
void  gui_destroy_font(void *);

void gui_widget_calc_rect(struct ne_widget *w);

#endif /* _NE_GUI_INTERNAL_H_ */

