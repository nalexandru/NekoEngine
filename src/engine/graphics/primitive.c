/* NekoEngine
 *
 * primitive.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Primitives
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

#include <engine/resource.h>

#include <graphics/mesh.h>
#include <graphics/vertex.h>
#include <graphics/graphics.h>
#include <graphics/primitive.h>

static struct ne_mesh *_primitives[PRIM_COUNT];
static struct ne_buffer *_prim_vtx_buffer, *_prim_idx_buffer;

ne_status
gfx_init_primitive(void)
{
	rt_array vtx;
	rt_array idx;
	struct ne_vertex v;
	uint32_t id = 0;
	struct ne_mesh_group grp;
	struct ne_buffer_create_info ci;

	memset(_primitives, 0x0, sizeof(_primitives));

	rt_array_init(&vtx, 40, sizeof(struct ne_vertex));
	rt_array_init(&idx, 40, sizeof(uint32_t));

	{ // Triangle
		_primitives[PRIM_TRIANGLE] = create_mesh("_builtin_triangle");
		if (!_primitives[PRIM_TRIANGLE])
			goto error;

		memset(&grp, 0x0, sizeof(grp));
		grp.idx_offset = idx.count;
		grp.vtx_offset = vtx.count;

		memset(&v, 0x0, sizeof(v));
		v.normal.x = 0.f, v.normal.y = 0.f; v.normal.z = -1.f;

		v.pos.x = -1.f; v.pos.y = -1.f; v.pos.z = 0.f;
		v.uv.x = 0.f; v.uv.y = 1.f;
		rt_array_add(&vtx, &v);

		v.pos.x = 1.f; v.pos.y = -1.f; v.pos.z = 0.f;
		v.uv.x = 1.f; v.uv.y = 1.f;
		rt_array_add(&vtx, &v);

		v.pos.x = -1.f; v.pos.y = 1.f; v.pos.z = 0.f;
		v.uv.x = 0.f; v.uv.y = 0.f;
		rt_array_add(&vtx, &v);

		id = 0;
		rt_array_add(&idx, &id); ++id;
		rt_array_add(&idx, &id); ++id;
		rt_array_add(&idx, &id); ++id;

		grp.idx_count = idx.count - grp.idx_offset;
		grp.vtx_count = vtx.count - grp.vtx_offset;

		rt_array_init(&_primitives[PRIM_TRIANGLE]->groups,
			1, sizeof(struct ne_mesh_group));
		rt_array_add(&_primitives[PRIM_TRIANGLE]->groups,
			&grp);
	}

	{ // Quad
		_primitives[PRIM_QUAD] = create_mesh("_builtin_quad");
		if (!_primitives[PRIM_QUAD])
			goto error;

		memset(&grp, 0x0, sizeof(grp));
		grp.idx_offset = idx.count;
		grp.vtx_offset = vtx.count;

		memset(&v, 0x0, sizeof(v));
		v.normal.x = 0.f, v.normal.y = 0.f; v.normal.z = -1.f;

		v.pos.x = -1.f; v.pos.y = -1.f; v.pos.z = 0.f;
		v.uv.x = 0.f; v.uv.y = 1.f;
		rt_array_add(&vtx, &v);

		v.pos.x = 1.f; v.pos.y = -1.f; v.pos.z = 0.f;
		v.uv.x = 1.f; v.uv.y = 1.f;
		rt_array_add(&vtx, &v);

		v.pos.x = -1.f; v.pos.y = 1.f; v.pos.z = 0.f;
		v.uv.x = 0.f; v.uv.y = 0.f;
		rt_array_add(&vtx, &v);

		v.pos.x = -1.f; v.pos.y = 1.f; v.pos.z = 0.f;
		v.uv.x = 0.f; v.uv.y = 0.f;
		rt_array_add(&vtx, &v);

		id = 0;
		rt_array_add(&idx, &id); ++id;
		rt_array_add(&idx, &id); ++id;
		rt_array_add(&idx, &id); ++id;
		rt_array_add(&idx, &id); ++id;
		rt_array_add(&idx, &id); ++id;
		rt_array_add(&idx, &id); ++id;

		grp.idx_count = idx.count - grp.idx_offset;
		grp.vtx_count = vtx.count - grp.vtx_offset;

		rt_array_init(&_primitives[PRIM_QUAD]->groups,
			1, sizeof(struct ne_mesh_group));
		rt_array_add(&_primitives[PRIM_QUAD]->groups,
			&grp);
	}

	_primitives[PRIM_CUBE] = res_load("/system/cube.nemesh", RES_MESH);
	if (!_primitives[PRIM_CUBE])
		goto error;

	_primitives[PRIM_PYRAMID] = res_load("/system/pyramid.nemesh", RES_MESH);
	if (!_primitives[PRIM_PYRAMID])
		goto error;

	_primitives[PRIM_SPHERE] = res_load("/system/sphere.nemesh", RES_MESH);
	if (!_primitives[PRIM_SPHERE])
		goto error;

	_primitives[PRIM_CONE] = res_load("/system/cone.nemesh", RES_MESH);
	if (!_primitives[PRIM_CONE])
		goto error;

	_primitives[PRIM_CYLINDER] = res_load("/system/cylinder.nemesh", RES_MESH);
	if (!_primitives[PRIM_CYLINDER])
		goto error;

	memset(&ci, 0x0, sizeof(ci));

	ci.access = NE_GPU_LOCAL;
	ci.offset = 0;

	ci.usage = NE_TRANSFER_DST | NE_VERTEX_BUFFER;
	ci.size = rt_array_byte_size(&vtx);
	_prim_vtx_buffer = gfx_create_buffer(&ci, vtx.data, 0, ci.size);
	if (!_prim_vtx_buffer)
		goto error;

	ci.usage = NE_TRANSFER_DST | NE_INDEX_BUFFER;
	ci.size = rt_array_byte_size(&idx);
	_prim_idx_buffer = gfx_create_buffer(&ci, idx.data, 0, ci.size);
	if (!_prim_idx_buffer)
		goto error;

	for (uint32_t i = 0; i < PRIM_COUNT; ++i) {
		if (!_primitives[i]->vtx_buffer)
			_primitives[i]->vtx_buffer = _prim_vtx_buffer;

		if (!_primitives[i]->idx_buffer)
			_primitives[i]->idx_buffer = _prim_idx_buffer;
	}

	return NE_OK;

error:
	rt_array_release(&idx);
	rt_array_release(&vtx);

	for (uint32_t i = 0; i < PRIM_COUNT; ++i) {
		if (!_primitives[i])
			continue;

		rt_array_release(&_primitives[i]->groups);
		rt_string_release(&_primitives[i]->name);

		free(_primitives[i]);
	}

	if (_prim_vtx_buffer)
		gfx_destroy_buffer(_prim_vtx_buffer);

	if (_prim_idx_buffer)
		gfx_destroy_buffer(_prim_idx_buffer);

	return NE_NO_MEMORY;
}

struct ne_mesh *
gfx_primitive(enum ne_primitive p)
{
	return _primitives[p];
}

void
gfx_release_primitive(void)
{
	for (uint32_t i = 0; i < PRIM_COUNT; ++i) {
		if (_primitives[i]->vtx_buffer == _prim_vtx_buffer) {
			rt_array_release(&_primitives[i]->groups);
			rt_string_release(&_primitives[i]->name);

			free(_primitives[i]);
		} else {
			res_unload(_primitives[i], RES_MESH);
		}
	}

	if (_prim_vtx_buffer)
		gfx_destroy_buffer(_prim_vtx_buffer);

	if (_prim_idx_buffer)
		gfx_destroy_buffer(_prim_idx_buffer);
}
