#include <metal_stdlib>
#include <simd/simd.h>

#include "Material.h"
#include "ShaderTypes.h"

using namespace metal;

struct VsOutput
{
	float4 position [[position, invariant]];
	float2 uv;
	float3 normal;
//	float3 tangent;
//	float3 biTangent;
	float3 vPos;
};

struct DrawInfo
{
	NE_BUFFER(vertex);
	NE_BUFFER(material);
	NE_BUFFER(instance);
};

struct VertexD
{
	float3 position [[attribute(0)]];
	float3 normal [[attribute(1)]];
	//float3 tangent [[attribute(2)]];
	float2 uv [[attribute(2)]];
//	float4 color [[attribute(4)]];
};

vertex struct VsOutput
Depth_VS(uint vertexId [[vertex_id]],
		 constant struct ShaderArguments *args [[ buffer(0) ]],
		 constant struct DrawInfo *drawInfo [[ buffer(1) ]],
		 struct VertexD vtx [[stage_in]])
{
	struct VsOutput out;

	constant ModelInstance *inst = (constant ModelInstance *)(args->buffers[drawInfo->instanceBuffer] + drawInfo->instanceOffset);

	const float3x3 normalMat = float3x3(inst->normal.columns[0].xyz,
										inst->normal.columns[1].xyz,
										inst->normal.columns[2].xyz);
	const float4 v = float4(vtx.position, 1.0);
	const float4 pos = inst->model * v;

//	out.tangent = normalize(normalMat * vtx.tangent);
	out.normal = normalize(normalMat * vtx.normal);
//	out.biTangent = normalize(normalMat * cross(out.tangent, out.normal));
	out.uv = vtx.uv;
	out.position = inst->mvp * v;

	out.vPos = pos.xyz / pos.w;

	return out;
}

fragment float4
Depth_FS(struct VsOutput in [[stage_in]],
		 constant struct ShaderArguments *args [[ buffer(0) ]],
		 constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	constant struct Material *mat = (constant struct Material *)(args->buffers[drawInfo->materialBuffer] + drawInfo->materialOffset);

	if (!mat->normalMap)
		return float4(normalize(in.normal), 1.0);

	float3 texnm = args->textures[mat->normalMap].sample(args->samplers[0], in.uv).xyz;
	texnm = normalize(texnm * float3(2.0) - float3(1.0));

	const float3 q1 = dfdx(in.vPos);
	const float3 q2 = dfdy(in.vPos);
	const float2 st1 = dfdx(in.uv);
	const float2 st2 = dfdy(in.uv);

	const float3 n = normalize(in.normal);
	const float3 t = normalize(q1 * st2.y - q2 * st1.y);
	const float3 b = -normalize(cross(n, t));
	const float3x3 tbn = float3x3(t, b, n);

	return float4(tbn * texnm, 1.0);
}

/* NekoEngine
 *
 * Depth.metal
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
