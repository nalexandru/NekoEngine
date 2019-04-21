/* NekoEngine
 *
 * mesh.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Mesh
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

#ifndef _NE_GRAPHICS_MESH_H_
#define _NE_GRAPHICS_MESH_H_

#include <stdint.h>
#include <stdbool.h>

#include <runtime/runtime.h>

#include <engine/math.h>
#include <ecs/component.h>
#include <graphics/material.h>

struct ne_mesh_group
{
	uint32_t vtx_offset;
	uint32_t vtx_count;
	uint32_t idx_offset;
	uint32_t idx_count;
};

struct ne_mesh_data
{
	kmMat4 model;
	kmMat4 mvp;
	kmMat4 normal;
	uint32_t id;
};

struct ne_mesh
{
	rt_string name;
	rt_array groups;
	struct ne_buffer *vtx_buffer;
	struct ne_buffer *idx_buffer;
	bool dynamic;
};

struct ne_drawable_mesh_comp
{
	NE_COMPONENT;

	const struct ne_mesh *mesh;
	rt_array materials;

	struct ne_mesh_data data;
	struct ne_buffer *data_buffer;

	// Reserved for the graphics subsyste,
	uint64_t private[4];
};

struct ne_mesh	*create_mesh(const char *name);
ne_status	 upload_mesh(struct ne_mesh *mesh);
void		 destroy_mesh(void *mesh);

#define DRAWABLE_MESH_COMP_TYPE		"ne_drawable_mesh_comp"

#ifdef _NE_ENGINE_INTERNAL_

void		*load_mesh(const char *path);

ne_status	 init_drawable_mesh_comp(void *, const void **);
void		 release_drawable_mesh_comp(void *);

NE_REGISTER_COMPONENT(DRAWABLE_MESH_COMP_TYPE, struct ne_drawable_mesh_comp, init_drawable_mesh_comp, release_drawable_mesh_comp)

#endif /* _NE_ENGINE_INTERNAL_ */

#endif /* _NE_GRAPHICS_MESH_H_ */

