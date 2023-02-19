#include <metal_stdlib>
#include <metal_atomic>

using namespace metal;

#define SA_WRITABLE_BUFFERS
#include "ShaderTypes.h"

struct ComputeArgs
{
	NE_BUFFER(scene);
	NE_BUFFER(visibleLightIndices);
	float4x4 view;
	uint depthMap;
	uint threadCount;
};

kernel void
LightCulling_CS(texture2d<float, access::read> depthBuffer [[texture(0)]],
			 device struct ShaderArguments *args [[ buffer(0) ]],
			 constant struct ComputeArgs *computeArgs [[ buffer(1) ]],
			 uint2 location [[thread_position_in_grid]],
			 uint2 item [[thread_position_in_threadgroup]],
			 uint2 tile [[threadgroup_position_in_grid]],
			 uint2 tileCount [[threadgroups_per_grid]],
			 uint id [[thread_index_in_threadgroup]])
{
	threadgroup atomic_uint minDepthI, maxDepthI, visibleLights;
	uint index = tile.y * tileCount.x + tile.x;

	device int32_t *visibleLightIndices = (device int32_t *)(args->buffers[computeArgs->visibleLightIndicesBuffer] + computeArgs->visibleLightIndicesOffset);
	device struct Scene *scn = (device struct Scene *)(args->buffers[computeArgs->sceneBuffer] + computeArgs->sceneOffset);

	matrix<float, 4> viewProjection;
	matrix<float, 4> inverseProjection;
	
	if (id == 0) {
		atomic_store_explicit(&minDepthI, 0, memory_order_relaxed);
		atomic_store_explicit(&maxDepthI, 0xFFFFFFFF, memory_order_relaxed);
		atomic_store_explicit(&visibleLights, 0, memory_order_relaxed);
		viewProjection = scn->viewProjection;
		inverseProjection = scn->inverseProjection;
	}

	threadgroup_barrier(mem_flags::mem_threadgroup);
	
	float maxDepth, minDepth;
	float depth = 1.f - args->textures[computeArgs->depthMap].read(ushort2(location)).r;
	depth = 1.0 / (depth * inverseProjection[2][3] + inverseProjection[3][3]);

	uint depthI = (uint)depth;

	atomic_fetch_min_explicit(&minDepthI, depthI, memory_order_relaxed);
	atomic_fetch_max_explicit(&maxDepthI, depthI, memory_order_relaxed);
	
	threadgroup_barrier(mem_flags::mem_threadgroup);

	threadgroup float4 frustum[6];
	if (id == 0) {
		minDepth = as_type<float>(atomic_load_explicit(&minDepthI, memory_order_relaxed));
		maxDepth = as_type<float>(atomic_load_explicit(&maxDepthI, memory_order_relaxed));

		float2 neg = (2.0 * float2(tile)) / float2(tileCount);
		float2 pos = (2.0 * float2(tile + uint2(1, 1))) / float2(tileCount);

		frustum[0]  = float4(1.0, 0.0, 0.0, 1.0 - neg.x) * viewProjection;
		frustum[0] /= length(frustum[0].xyz);
		frustum[1]  = float4(-1.0, 0.0, 0.0, -1.0 + pos.x) * viewProjection;
		frustum[1] /= length(frustum[1].xyz);
		frustum[2]  = float4(0.0, -1.0, 0.0, 1.0 - neg.y) * viewProjection;
		frustum[2] /= length(frustum[2].xyz);
		frustum[3]  = float4(0.0, 1.0, 0.0, -1.0 + pos.y) * viewProjection;
		frustum[3] /= length(frustum[3].xyz);
		frustum[4]  = float4(0.0, 0.0, 1.0, -minDepth) * computeArgs->view;
		frustum[4] /= length(frustum[4].xyz);
		frustum[5]  = float4(0.0, 0.0, -1.0, maxDepth) * computeArgs->view;
		frustum[5] /= length(frustum[5].xyz);
	}

	threadgroup_barrier(mem_flags::mem_threadgroup);

	// TODO: support more than 4096 (visible ?) lights in a scene
	threadgroup int32_t indices[4096];

	device struct Light *lights = (device struct Light *)&scn->lightStart;
	const uint threadCount = computeArgs->threadCount;
	const uint passCount = (scn->lightCount + threadCount - 1) / threadCount;
	for (uint i = 0; i < passCount; ++i) {
		uint lightIndex = i * threadCount + id;
		if (lightIndex >= scn->lightCount) // light count
			break;

		struct Light l = lights[lightIndex];
		float4 lPos = float4(l.x, l.y, l.z, 1.0);

		if (l.type != LT_DIRECTIONAL) {
			if (dot(frustum[0], lPos) < -l.outerRadius)
				continue;
			
			if (dot(frustum[1], lPos) < -l.outerRadius)
				continue;
			
			if (dot(frustum[2], lPos) < -l.outerRadius)
				continue;
			
			if (dot(frustum[3], lPos) < -l.outerRadius)
				continue;
			
			if (dot(frustum[4], lPos) < -l.outerRadius)
				continue;
			
			if (dot(frustum[5], lPos) < -l.outerRadius)
				continue;
		}

		uint offset = atomic_fetch_add_explicit(&visibleLights, 1, memory_order_relaxed);
		indices[offset] = lightIndex;
	}

	threadgroup_barrier(mem_flags::mem_threadgroup);
	
	if (id == 0) {
		uint offset = index * scn->lightCount;
		uint vl = atomic_load_explicit(&visibleLights, memory_order_relaxed);

		for (uint i = 0; i < vl; ++i)
			visibleLightIndices[offset + i] = indices[i];
		
		if (vl != scn->lightCount)
			visibleLightIndices[offset + vl] = -1;
	}
}

/* NekoEngine
 *
 * LightCulling.metal
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
