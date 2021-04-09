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

struct ShaderArguments
{
	array<sampler, 3> samplers [[ id(0) ]];
	array<texture2d<float>, 65535> textures [[ id(3) ]];
	array<constant struct Vertex *, 65535> vertexBuffers [[ id(65538) ]];
};

vertex struct VsOutput
DefaultPBR_VS(uint vertexId [[vertex_id]],
			  constant struct ShaderArguments *args [[ buffer(0) ]])
{
	struct Vertex vtx = args->vertexBuffers[0][vertexId];
	struct VsOutput out;
	
	out.position = float4(vtx.x, vtx.y, vtx.z, 1.0);
	out.color = float4(vtx.u, vtx.v, 1.0, 1.0);
	out.uv = float2(vtx.u, vtx.v);
	
	return out;
}

fragment float4
DefaultPBR_MR_FS(struct VsOutput in [[stage_in]],
				 constant struct ShaderArguments *args [[ buffer(0) ]])
{
//	return float4(1.0, 1.0, 1.0, 1.0);
	return args->textures[0].sample(args->samplers[0], in.uv);
}

fragment float4
DefaultPBR_SG_FS(struct VsOutput in [[stage_in]])
{
	return in.color;
}
