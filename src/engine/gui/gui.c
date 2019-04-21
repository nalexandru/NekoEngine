/* NekoEngine
 *
 * gui.c
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

#include <system/log.h>

#include <ecs/ecsys.h>
#include <gui/gui.h>
#include <gui/font.h>
#include <gui/widget.h>
#include <gui/gui_internal.h>
#include <engine/resource.h>
#include <graphics/graphics.h>

#define GUI_MODULE	"GUI"

struct ne_font *ne_system_font = NULL;

ne_status
gui_init(void)
{
	if (res_register_type(RES_FONT, gui_load_font,
		gui_destroy_font) != NE_OK) {
		log_entry(GUI_MODULE, LOG_CRITICAL,
			"Failed to register font resource handler");
		return NE_FAIL;
	}

	comp_type_id font_type = comp_get_type_id(NE_FONT_COMP);
	ecsys_register("font_reset_sys", ECSYS_GROUP_PRE_LOGIC,
			&font_type, 1, gui_reset_font_sys, true, 0);

	rt_array_init_ptr(&gui_widgets, 50);

	ne_system_font = res_load("/system/font.ttf", RES_FONT);
	if (!ne_system_font) {
		log_entry(GUI_MODULE, LOG_CRITICAL,
			"Failed to load system font");
		return NE_FAIL;
	}

	return NE_OK;
}

void
gui_screen_resized(void)
{
	struct ne_widget* w = NULL;
	rt_array_foreach_ptr(w, &gui_widgets)
		gui_widget_calc_rect(w);
}

void
gui_release(void)
{
	rt_array_release(&gui_widgets);

	res_unload(ne_system_font, RES_FONT);
}

ne_status
gui_drawable_comp_create(void *comp,
	const void **args)
{
	return gfx_init_gui_drawable(comp);
}

void
gui_drawable_comp_destroy(void *comp)
{
	gfx_release_gui_drawable(comp);
}
