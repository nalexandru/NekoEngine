#include <metal_stdlib>
#include <simd/simd.h>

#include "ShaderTypes.h"

using namespace metal;

struct VsOutput
{
	float4 position [[position]];
	float4 color;
};

vertex struct VsOutput
DefaultPBR_VS(uint vertexId [[vertex_id]],
			 constant struct Vertex *vertices [[buffer(0)]])
{
	struct Vertex vtx = vertices[vertexId];
	struct VsOutput out;
	
	out.position = float4(vtx.x, vtx.y, vtx.z, 1.0);
	out.color = float4(1.0, 1.0, 1.0, 1.0);
	
	return out;
}

fragment float4
DefaultPBR_MR_FS(struct VsOutput in [[stage_in]])
{
	return in.color;
}

fragment float4
DefaultPBR_SG_FS(struct VsOutput in [[stage_in]])
{
	return in.color;
}
