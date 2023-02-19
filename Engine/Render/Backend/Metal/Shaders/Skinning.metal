#include <metal_stdlib>

using namespace metal;

#define SA_WRITABLE_BUFFERS
#include "ShaderTypes.h"

struct VertexWeight
{
	int i0, i1, i2, i3, i4, i5, i6;
	float w0, w1, w2, w3, w4, w5, w6;
	uint boneCount;
	uint reserved;
};

struct ComputeArgs
{
	NE_BUFFER(bone);
	NE_BUFFER(weight);
	NE_BUFFER(src);
	NE_BUFFER(dst);
};

kernel void
Skinning_CS(texture2d<float, access::read> depthBuffer [[texture(0)]],
			 device struct ShaderArguments *args [[ buffer(0) ]],
			 constant struct ComputeArgs *computeArgs [[ buffer(1) ]],
			 uint3 globalId [[thread_position_in_grid]])
{
	uint vertexId = globalId.x;
	
	const struct Vertex vtx = ((device struct Vertex *)(args->buffers[computeArgs->srcBuffer] + computeArgs->srcOffset))[vertexId];
	const struct VertexWeight weights = ((device struct VertexWeight *)(args->buffers[computeArgs->weightBuffer] + computeArgs->weightOffset))[vertexId];
	device float4x4 *bones = (device float4x4 *)(args->buffers[computeArgs->boneBuffer] + computeArgs->boneOffset);
	device struct Vertex *dst = &((device struct Vertex *)(args->buffers[computeArgs->dstBuffer] + computeArgs->dstOffset))[vertexId];
	
	float4x4 xform = bones[weights.i0] * weights.w0;
	xform += bones[weights.i1] * weights.w1;
	xform += bones[weights.i2] * weights.w2;
	xform += bones[weights.i3] * weights.w3;
	xform += bones[weights.i4] * weights.w4;
	xform += bones[weights.i5] * weights.w5;
	xform += bones[weights.i6] * weights.w6;
	
	float4 pos = xform * float4(vtx.x, vtx.y, vtx.z, 1.0);
	dst->x = pos.x; dst->y = pos.y; dst->z = pos.z;
	
	float4 norm = xform * float4(vtx.nx, vtx.ny, vtx.nz, 1.0);
	dst->nx = norm.x; dst->ny = norm.y; dst->nz = norm.z;
	
	float4 tgt = xform * float4(vtx.tx, vtx.ty, vtx.tz, 1.0);
	dst->tx = tgt.x; dst->ty = tgt.y; dst->tz = tgt.z;
	
	dst->u = vtx.u; dst->v = vtx.v;
	dst->r = vtx.r; dst->g = vtx.g; dst->b = vtx.b; dst->a = vtx.a;
}

/* NekoEngine
 *
 * Skinning.metal
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
 * -----------------------------------------------------------------------------
 */
