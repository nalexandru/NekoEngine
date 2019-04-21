/* NekoEngine
 *
 * systems.c
 * Author: Alexandru Naiman
 *
 * NekoEngine OpenGL Renderer Systems
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

#include <gui/gui.h>
#include <scene/ecsys.h>

#include "glad.h"

struct gui_draw_info
{
	uint32_t vao;
	uint32_t vtx_count;
	uint32_t idx_count;
	uint32_t vbo;
	uint32_t ibo;
	bool initialized;
};

void
gui_update_system(double dt,
	void **comp)
{
	struct ne_gui_drawable_comp *draw = comp[0];
	struct gui_draw_info *info = (struct gui_draw_info *)&draw->private;
	
	if (!info->initialized) {
	//	glGenVertexArrays(1, &info->vao);
	//	glBindVertexArray(info->vao);

	//	glGenBuffers(1, &info->vbo);
		//glBindBuffer(GL_ARRAY_BUFFER, 

	} else {
	}
}

void
gui_draw_system(double dt,
	void **comp)
{
	struct ne_gui_drawable_comp *draw = comp[0];
	struct gui_draw_info *info = (struct gui_draw_info *)&draw->private;
	
	if (!info->initialized)
		return;


}

ne_status
register_systems(void)
{
	return NE_OK;
}

void
unregister_systems(void)
{
	//
}

