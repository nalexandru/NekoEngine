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

vertex struct VsOutput
DefaultPBR_VS(uint vertexId [[vertex_id]],
			 constant struct Vertex *vertices [[buffer(0)]])
{
	struct Vertex vtx = vertices[vertexId];
	struct VsOutput out;
	
	out.position = float4(vtx.x, vtx.y, vtx.z, 1.0);
	out.color = float4(vtx.u, vtx.v, 1.0, 1.0);
	out.uv = float2(vtx.u, vtx.v);
	
	return out;
}

fragment float4
DefaultPBR_MR_FS(struct VsOutput in [[stage_in]],
				 array<texture2d<float>, 16> colorTextures [[texture(0)]],
				 sampler sceneSampler [[sampler(0)]])
{
	return colorTextures[0].sample(sceneSampler, in.uv);
}

fragment float4
DefaultPBR_SG_FS(struct VsOutput in [[stage_in]])
{
	return in.color;
}
