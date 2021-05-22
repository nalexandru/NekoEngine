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
	array<constant uint8_t *, 65535> vertexBuffers [[ id(65538) ]];
};

struct DrawInfo
{
	uint vertexBuffer;
	uint __padding;
	uint materialBuffer;
	uint materialOffset;
};

vertex struct VsOutput
DefaultPBR_VS(uint vertexId [[vertex_id]],
			  constant struct ShaderArguments *args [[ buffer(0) ]],
			  constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	struct Vertex vtx = ((constant struct Vertex *)args->vertexBuffers[drawInfo->vertexBuffer])[vertexId];
	struct VsOutput out;
	
	out.position = float4(vtx.x, vtx.y, vtx.z, 1.0);
	out.color = float4(vtx.u, vtx.v, 1.0, 1.0);
	out.uv = float2(vtx.u, vtx.v);
	
	return out;
}

fragment float4
DefaultPBR_MR_FS(struct VsOutput in [[stage_in]],
				 constant struct ShaderArguments *args [[ buffer(0) ]],
				 constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	constant struct Material *mat = (constant struct Material *)(args->vertexBuffers[drawInfo->materialBuffer] + drawInfo->materialOffset);
	
	return args->textures[mat->diffuseMap].sample(args->samplers[0], in.uv);
}

fragment float4
DefaultPBR_SG_FS(struct VsOutput in [[stage_in]])
{
	return in.color;
}
