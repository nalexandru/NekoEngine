/* NekoEngine
 *
 * gl1_gfx.h
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

#ifndef _NE_GL1_GRAPHICS_H_
#define _NE_GL1_GRAPHICS_H_

#include <stdint.h>
#include <stdbool.h>

#include <system/platform.h>

#include <engine/status.h>

#if defined(SYS_PLATFORM_APPLE)
	#include <OpenGL/OpenGL.h>
#elif defined(SYS_PLATFORM_WINDOWS)
	#include <windows.h>
	#include <GL/gl.h>

	#define GL_SRGB 0x8C40
	#define GL_COMBINE 0x8570
	#define GL_COMBINE_RGB 0x8571
	#define GL_PRIMARY_COLOR 0x8577
	#define GL_COMBINE_ALPHA 0x8572
	#define GL_SRC0_RGB 0x8580
	#define GL_SRC0_ALPHA 0x8588
	#define GL_OPERAND0_RGB 0x8590
	#define GL_OPERAND0_ALPHA 0x8598
#else
	#include <GL/gl.h>
#endif

struct ne_mesh;
struct ne_material;
struct ne_drawable_mesh_comp;

enum gl1gfx_view_matrix_type
{
	MAT_NONE,
	MAT_PERSPECTIVE,
	MAT_GUI
};

extern struct ne_gfx_module		gl1gfx_module;
extern enum gl1gfx_view_matrix_type	gl1gfx_view_matrix;

ne_status	 gfx_sys_init(bool debug);
void		 gfx_sys_swap_interval(int swi);
void		 gfx_sys_swap_buffers();
void		 gfx_sys_destroy(void);

ne_status	 gl1gfx_init_render(void);
void		 gl1gfx_render_scene(void);
void		 gl1gfx_release_render(void);

ne_status	 gl1gfx_init_drawable_mesh(struct ne_drawable_mesh_comp *comp);
void		 gl1gfx_release_drawable_mesh(struct ne_drawable_mesh_comp *comp);
void		 gl1gfx_add_mesh(struct ne_drawable_mesh_comp *comp, size_t group);

struct ne_light *gl1gfx_create_light(void);
void		 gl1gfx_destroy_light(struct ne_light *);

#endif /* _NE_GL1_GRAPHICS_H_ */

