#include <metal_stdlib>
#include <simd/simd.h>

#include "Material.h"
#include "ShaderTypes.h"

using namespace metal;

struct VsOutput
{
	float4 position [[position, invariant]];
	float2 uv;
};

struct AOData
{
	uint kernelSize;
	float radius;
	float powerExponent;
	float threshold;
	float bias;
	uint noiseSize;
	uint noiseTexture;
	float padding;
};

struct AOInfo
{
	float4x4 inverseView;
	NE_BUFFER(data);
	NE_BUFFER(scene);
	uint depthTexture;
};

struct BlurInfo
{
	uint ssaoTexture;
	float texelSizeX;
	float texelSizeY;
};

constant float blurOffsets[4] = { -2.0, -1.0, 0.0, 1.0 };

float3
posFromDepth(constant struct ShaderArguments *sa, constant struct Scene *scn, uint depthTexture, float2 uv)
{
	float4 projPos = float4(uv.x * 2.0 - 1.0, (1.0 - uv.y) * 2.0 - 1.0, sa->textures[depthTexture].sample(sa->samplers[0], uv).r, 1.0);
	float4 pos = scn->inverseProjection * projPos;
	return (pos.xyz / pos.w);
}

vertex struct VsOutput
Fullscreen_VS(uint vertexId [[vertex_id]],
			  constant struct ShaderArguments *args [[ buffer(0) ]],
			  constant struct AOInfo *info [[ buffer(1) ]])
{
	struct VsOutput out;
	
	out.uv = float2((vertexId << 1) & 2, vertexId & 2);
	out.position = float4(out.uv * 2.0 - 1.0, 0.0, 1.0);

	return out;
}

fragment float4
SSAO_FS(struct VsOutput in [[stage_in]],
		constant struct ShaderArguments *args [[ buffer(0) ]],
		constant struct AOInfo *info [[ buffer(1) ]],
		float4 wsNormal [[ color(1) ]])
{
	constant struct Scene *scn = (constant struct Scene *)(args->buffers[info->sceneBuffer] + info->sceneOffset);
	constant struct AOData *data = (constant struct AOData *)(args->buffers[info->dataBuffer] + info->dataOffset);
	constant float4 *krnl = (constant float4 *)(args->buffers[info->dataBuffer] + info->dataOffset + sizeof(constant struct AOData));
	
	float3 fragPos = posFromDepth(args, scn, info->depthTexture, in.uv);
	float3 normal = normalize((float4(wsNormal.xyz, 1.0) * info->inverseView).xyz);
	float3 rand = float3(args->textures[data->noiseTexture].sample(args->samplers[0], in.uv * data->noiseSize).rg, 0.0);
	float3 tangent = normalize(rand - normal * dot(rand, normal));
	float3 bitangent = cross(tangent, normal);
	float3x3 tbn = float3x3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < int(data->kernelSize); ++i) {
		if (dot(krnl[i].xyz, normal) < data->threshold)
			continue;

		float3 samplePos = tbn * krnl[i].xyz;
		samplePos = fragPos + samplePos * data->radius;

		float4 offset = float4(samplePos, 1.0);
		offset = scn->projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = posFromDepth(args, scn, info->depthTexture, offset.xy).z;
		float rangeCheck = smoothstep(0.0, 1.0, data->radius / abs(fragPos.z - sampleDepth));

		occlusion += (sampleDepth >= samplePos.z + data->bias ? 1.0 : 0.0) * rangeCheck;
	}

	return pow(1.0 - (occlusion / data->kernelSize), data->powerExponent);
}

fragment float4
SSAOBlur_FS(struct VsOutput in [[stage_in]],
		 constant struct ShaderArguments *args [[ buffer(0) ]],
		 constant struct BlurInfo *info [[ buffer(1) ]])
{
	float color = 0.0;
	float2 offset = float2(0.0);
	float2 texelSize = float2(info->texelSizeX, info->texelSizeY);
	uint ssaoTexture = info->ssaoTexture;

	offset = float2(blurOffsets[0], blurOffsets[0]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[0], blurOffsets[1]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[0], blurOffsets[2]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[0], blurOffsets[3]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;


	offset = float2(blurOffsets[1], blurOffsets[0]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[1], blurOffsets[1]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[1], blurOffsets[2]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[1], blurOffsets[3]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;


	offset = float2(blurOffsets[2], blurOffsets[0]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[2], blurOffsets[1]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[2], blurOffsets[2]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[2], blurOffsets[3]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;


	offset = float2(blurOffsets[3], blurOffsets[0]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[3], blurOffsets[1]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[3], blurOffsets[2]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	offset = float2(blurOffsets[3], blurOffsets[3]) * texelSize;
	color += args->textures[ssaoTexture].sample(args->samplers[0], in.uv + offset).r;

	return color / 16.0;
}

/* NekoEngine
 *
 * SSAO.metal
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
