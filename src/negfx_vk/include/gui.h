/* NekoEngine
 *
 * gui.h
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem GUI
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

#ifndef _VKGFX_GUI_H_
#define _VKGFX_GUI_H_

#include <vulkan/vulkan.h>
#include <engine/status.h>

struct ne_font;

ne_status		vkgfx_init_gui(void);

void			vkgfx_gui_update(void);

VkCommandBuffer		vkgfx_gui_build_cb(uint32_t id);
void			gui_update_system(double dt, void **comp);
void			gui_draw_system(double dt, void **comp);

ne_status		vkgfx_register_font(struct ne_font *);
void			vkgfx_unregister_font(struct ne_font *);

ne_status		vkgfx_init_gui_drawable(struct ne_gui_drawable_comp *draw);
void			vkgfx_free_gui_drawable(struct ne_gui_drawable_comp *draw);

void			vkgfx_release_gui(void);

#endif /* _VKGFX_GUI_H_ */

