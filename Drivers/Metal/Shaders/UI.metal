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
