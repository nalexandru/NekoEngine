/* NekoEngine
 *
 * nmesh.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Mesh Loader
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

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <system/system.h>
#include <system/compat.h>
#include <runtime/runtime.h>

#include <graphics/mesh.h>
#include <graphics/vertex.h>

#define MESH_HEADER	"NMESH2B"
#define MESH_FOOTER	"ENDMESH"

#define READ_UINT32(dst, src, pos, size)		\
	assert(pos + sizeof(uint32_t) < size);		\
	memcpy(&dst, data, sizeof(uint32_t));		\
	if (sys_is_big_endian()) rt_swap_uint32(&dst);	\
	data += sizeof(uint32_t);			\
	pos += sizeof(uint32_t);

#define SWAP_VEC2(v)					\
	rt_swap_uint32((uint32_t *)&v.x);		\
	rt_swap_uint32((uint32_t *)&v.y)

#define SWAP_VEC3(v)					\
	rt_swap_uint32((uint32_t *)&v.x);		\
	rt_swap_uint32((uint32_t *)&v.y);		\
	rt_swap_uint32((uint32_t *)&v.z)

ne_status
asset_load_nmesh(
	const uint8_t *data,
	uint64_t data_size,
	rt_array *vertices,
	rt_array *indices,
	rt_array *groups)
{
	uint32_t vtx_count = 0, idx_count = 0, grp_count = 0;
	uint64_t pos = 0;
	size_t size = 0;
	char guard[8];

	assert(pos + sizeof(char) * 7 < data_size);
	memcpy(guard, data, sizeof(char) * 7);
	data += 7;
	pos += 7;
	guard[7] = 0x0;

	if (strncmp(guard, MESH_HEADER, 7))
		return NE_INVALID_HEADER;

	READ_UINT32(vtx_count, data, pos, data_size);
	rt_array_init(vertices, vtx_count, sizeof(struct ne_vertex));
	rt_array_fill(vertices);

	size = sizeof(struct ne_vertex) * vtx_count;
	assert(pos + size < data_size);
	memcpy(vertices->data, data, size);
	data += size; pos += size;

	READ_UINT32(idx_count, data, pos, data_size);
	rt_array_init(indices, idx_count, sizeof(uint32_t));
	rt_array_fill(indices);

	size = sizeof(uint32_t) * idx_count;
	assert(pos + size < data_size);
	memcpy(indices->data, data, size);
	data += size; pos += size;

	READ_UINT32(grp_count, data, pos, data_size);
	rt_array_init(groups, grp_count, sizeof(struct ne_mesh_group));

	for (uint32_t i = 0; i < grp_count; ++i) {
		struct ne_mesh_group *grp = rt_array_create(groups);
		memset(grp, 0x0, sizeof(*grp));

		READ_UINT32(grp->vtx_offset, data, pos, data_size);
		READ_UINT32(grp->vtx_count, data, pos, data_size);
		READ_UINT32(grp->idx_offset, data, pos, data_size);
		READ_UINT32(grp->idx_count, data, pos, data_size);

		// FIXME
		grp->vtx_offset = 0;
	}

	assert(pos + (sizeof(char) * 7) == data_size);
	memcpy(guard, data, sizeof(char) * 7);
	guard[7] = 0x0;

	if (strncmp(guard, MESH_FOOTER, 7))
		return NE_INVALID_MESH;

	if (!sys_is_big_endian())
		return NE_OK;

	for (uint32_t i = 0; i < vtx_count; ++i) {
		struct ne_vertex *vtx = rt_array_get(vertices, i);

		SWAP_VEC3(vtx->pos);
		SWAP_VEC2(vtx->uv);
		SWAP_VEC3(vtx->normal);
		SWAP_VEC3(vtx->tangent);
	}

	for (uint32_t i = 0; i < idx_count; ++i)
		rt_swap_uint32(rt_array_get(indices, i));

	return NE_OK;
}

