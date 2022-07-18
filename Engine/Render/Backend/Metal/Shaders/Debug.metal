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
