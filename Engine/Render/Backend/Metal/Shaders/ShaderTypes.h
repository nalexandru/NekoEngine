#ifndef ShaderTypes_h
#define ShaderTypes_h

#include <metal_stdlib>
using namespace metal;

#define NE_BUFFER(x) \
	uint32_t x ## Buffer; \
	uint32_t x ## Offset

struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
	float r, g, b, a;
};

struct VertexO
{
	float3 position [[attribute(0)]];
	float2 uv [[attribute(1)]];
	float4 color [[attribute(2)]];
};

struct VertexT
{
	float3 position [[attribute(0)]];
	float2 uv [[attribute(1)]];
	float4 color [[attribute(2)]];
	float3 normal [[attribute(3)]];
};

struct UIVertex
{
//	float4 posAndUV [[attribute[0]]]
	float x, y, u, v;
	float4 color;
};

struct VertexBones
{
	int4 indices [[attribute(5)]];
	float4 weights [[attribute(6)]];
	int boneCount [[attribute(7)]];
};

struct Light
{
	float x, y, z;
	uint type;

	float4 direction;
	float4 color;

	float innerRadius, outerRadius;
	float innerCutoff, outerCutoff;
};

struct Scene
{
	float4x4 viewProjection;
	float4x4 inverseProjection;
	float4 cameraPosition;

	float4 sunPosition;
	uint4 enviornmentMaps;

	uint lightCount;
	uint xTileCount;

	NE_BUFFER(instances);

	float exposure;
	float gamma;
	float invGamma;
	uint sampleCount;

	Light lightStart;
};

struct ModelInstance
{
	float4x4 mvp;
	float4x4 model;
	float4x4 normal;
	NE_BUFFER(vertex);
	NE_BUFFER(material);
};

#define LT_DIRECTIONAL	0
#define LT_POINT		1
#define LT_SPOT			2

struct ShaderArguments
{
	const array<sampler, 3> samplers [[ id(0) ]];

#ifndef SA_CUBE_TEXTURES
	const array<texture2d<float>, 65535> textures [[ id(3) ]];
#else
	const array<texturecube<float>, 65535> cubeTextures [[ id(3) ]];
#endif

#ifdef SA_WRITABLE_BUFFERS
	const array<device uint8_t *, 65535> buffers [[ id(65538) ]];
#else
	const array<constant uint8_t *, 65535> buffers [[ id(65538) ]];
#endif
};

#endif /* ShaderTypes_h */

/* NekoEngine
 *
 * ShaderTypes.h
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
