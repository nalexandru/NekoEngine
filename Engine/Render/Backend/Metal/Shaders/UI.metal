#include <metal_stdlib>
#include <simd/simd.h>

#include "ShaderTypes.h"

using namespace metal;

struct VsOutput
{
	float4 position [[position]];
	float4 color;
	float2 uv;
};

struct DrawInfo
{
	uint32_t vertexBuffer;
	uint32_t vertexOffset;
	uint32_t texture;
	uint32_t __padding;
	float4x4 mvp;
};

vertex struct VsOutput
UI_VS(uint vertexId [[vertex_id]],
			  constant struct ShaderArguments *args [[ buffer(0) ]],
			  constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	struct UIVertex vtx = ((constant struct UIVertex *)(args->buffers[drawInfo->vertexBuffer] + drawInfo->vertexOffset))[vertexId];
	struct VsOutput out;
	
	out.position = drawInfo->mvp * float4(vtx.x, vtx.y, 0.0, 1.0);
	out.color = vtx.color;
	out.uv = float2(vtx.u, vtx.v);
	
	return out;
}

fragment float4
UI_FS(struct VsOutput in [[stage_in]],
				 constant struct ShaderArguments *args [[ buffer(0) ]],
				 constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	return float4(in.color.xyz, args->textures[drawInfo->texture].sample(args->samplers[0], in.uv).r);
}

/* NekoEngine
 *
 * UI.metal
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
