/* NekoEngine
 *
 * gui.h
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

#ifndef _NE_GUI_H_
#define _NE_GUI_H_

#include <system/platform.h>

#include <gui/font.h>
#include <gui/widget.h>
#include <engine/math.h>
#include <engine/status.h>
#include <ecs/component.h>
#include <graphics/vertex.h>
#include <engine/components.h>
#include <graphics/texture.h>

struct ne_gui_drawable_comp
{
	NE_COMPONENT;

	uint64_t private[8];
	struct ne_gui_vertex vertices[4];
	struct ne_texture *texture;
	bool dirty, visible;

} ne_gui_drawable_comp;

MIWA_EXPORT extern struct ne_font *ne_system_font;

#ifdef _NE_ENGINE_INTERNAL_

ne_status	gui_init(void);
void		gui_screen_resized(void);
void		gui_release(void);

ne_status	gui_drawable_comp_create(void *comp, const void **args);
void		gui_drawable_comp_destroy(void *comp);

NE_REGISTER_COMPONENT(GUI_DRAWABLE_COMP_TYPE, struct ne_gui_drawable_comp, gui_drawable_comp_create, gui_drawable_comp_destroy)

#endif /* _NE_ENGINE_INTERNAL */

#endif /* _NE_GUI_H_ */

