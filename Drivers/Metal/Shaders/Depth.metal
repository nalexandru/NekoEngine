#include <metal_stdlib>
#include <simd/simd.h>

#include "ShaderTypes.h"

using namespace metal;

struct VsOutput
{
	float4 position [[position]];
	float3 normal;
	float2 uv;
};

struct ShaderArguments
{
	array<sampler, 3> samplers [[ id(0) ]];
	array<texture2d<float>, 65535> textures [[ id(3) ]];
	array<constant uint8_t *, 65535> vertexBuffers [[ id(65538) ]];
};

struct DrawInfo
{
	uint vertexBuffer;
	uint vertexOffset;
	uint materialBuffer;
	uint materialOffset;
	float4x4 mvp;
};

vertex struct VsOutput
Depth_VS(uint vertexId [[vertex_id]],
		 constant struct ShaderArguments *args [[ buffer(0) ]],
		 constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	struct Vertex vtx = ((constant struct Vertex *)(args->vertexBuffers[drawInfo->vertexBuffer] + drawInfo->vertexOffset))[vertexId];
	struct VsOutput out;

	out.position = drawInfo->mvp * float4(vtx.x, vtx.y, vtx.z, 1.0);
	out.normal = float3(vtx.nx, vtx.ny, vtx.nz);
	out.uv = float2(vtx.u, vtx.v);

	return out;
}

fragment float4
Depth_FS(struct VsOutput in [[stage_in]],
		 constant struct ShaderArguments *args [[ buffer(0) ]],
		 constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
//	constant struct Material *mat = (constant struct Material *)(args->vertexBuffers[drawInfo->materialBuffer] + drawInfo->materialOffset);
//	return args->textures[mat->diffuseMap].sample(args->samplers[0], in.uv);
	return float4(in.normal, 1.0);
}
