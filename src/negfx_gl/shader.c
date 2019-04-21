/* NekoEngine
 *
 * shader.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Shader
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

#include <runtime/runtime.h>

#include "glad.h"
#include <graphics/shader.h>

typedef struct ne_ub
{
	GLuint ubo;
	size_t offset;
	size_t size;
	uint32_t binding;
	uint32_t index;
} ne_ub;

typedef struct ne_ui
{
	rt_string location;
	rt_string name;
} ne_ui;

struct ne_shader
{
	GLint program;
	GLint shaders[6];
	ne_ub vs_buf[10];
	ne_ub fs_buf[10];
	uint8_t vs_buf_count;
	uint8_t fs_buf_count;
	uint8_t next_binding;
	rt_array uniform_info;
};

ne_shader *
shader_load(const char *path)
{
	return NULL;
}

void
shader_bind_ub(ne_shader *sh)
{
	//
}

void
shader_vs_ub_binding(ne_shader *sh,
	int loc,
	const char *name)
{
	//
}
void
shader_fs_ub_binding(ne_shader *sh,
	int loc,
	const char *name)
{
	//
}

void
shader_vs_ub_set(ne_shader *sh,
	int loc,
	uint64_t off,
	uint64_t size,
	void *buff)
{
	//
}

void
shader_fs_ub_set(ne_shader *sh,
	int loc,
	uint64_t off,
	uint64_t size,
	void *buff)
{
	//
}

void
shader_set_texture(ne_shader *sh,
	unsigned int loc,
	ne_texture *tex)
{
	//
}

void
shader_destroy(ne_texture *tex)
{
	//
}
