/* NekoEngine
 *
 * widget.h
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

#ifndef _NE_GUI_WIDGET_H_
#define _NE_GUI_WIDGET_H_

#include <gui/guidefs.h>
#include <engine/status.h>

struct ne_widget;
typedef struct ne_widget ne_widget;

typedef enum ne_gradient_type
{
	GRADIENT_VERTICAL,
	GRADIENT_HORIZONTAL
} ne_gradient_type;

ne_widget	*gui_widget_create(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void		 gui_widget_set_position(ne_widget *w, uint16_t x, uint16_t y);
void		 gui_widget_set_size(ne_widget *w, uint16_t width, uint16_t height);
void		 gui_widget_set_enable(ne_widget *w, bool enable);
void		 gui_widget_set_visibile(ne_widget *w, bool visible);
void		 gui_widget_set_handler(ne_widget *w, ne_gui_evt event, gui_handler handler);
void		 gui_widget_set_bg_color(ne_widget *w, float color[4]);
void		 gui_widget_set_bg_gradient(ne_widget *w, float start[4], float end[4], ne_gradient_type type);
void		 gui_widget_destroy(ne_widget *w);

#endif /* _NE_GUI_WIDGET_H_ */

