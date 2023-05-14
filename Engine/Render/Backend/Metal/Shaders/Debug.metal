#include <metal_stdlib>
#include <simd/simd.h>

#include "Material.h"
#include "ShaderTypes.h"

using namespace metal;

struct DebugVertex
{
	float x, y, z;
};

struct VsOutput
{
	float4 position [[position, invariant]];
};

struct DrawInfo
{
	float4x4 mvp;
	NE_BUFFER(vertex);
};

vertex struct VsOutput
DebugBounds_VS(uint vertexId [[vertex_id]],
			   constant struct ShaderArguments *args [[ buffer(0) ]],
			   constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	struct VsOutput out;

	const struct DebugVertex vtx = ((constant struct DebugVertex *)(args->buffers[drawInfo->vertexBuffer] + drawInfo->vertexOffset))[vertexId];
	out.position = drawInfo->mvp * float4(vtx.x, vtx.y, vtx.z, 1.0);

	return out;
}

fragment float4
DebugBounds_FS(struct VsOutput in [[stage_in]],
			   constant struct ShaderArguments *args [[ buffer(0) ]],
			   constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	return float4(1.0);
}

/* NekoEngine
 *
 * Debug.metal
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
