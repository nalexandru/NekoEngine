#include <metal_stdlib>
using namespace metal;

//#define SA_CUBE_TEXTURES

#include "ShaderTypes.h"
#include "Tonemap.h"

struct SkyVertex
{
	float x, y, z;
};

struct VsOutput
{
	float4 position [[position]];
	float3 uv;
};

struct DrawInfo
{
	uint32_t texture;
	float exposure;
	float gamma;
	float invGamma;
	float4x4 mvp;
	NE_BUFFER(vertex);
};

vertex struct VsOutput
Sky_VS(uint vertexId [[vertex_id]],
		 constant struct ShaderArguments *args [[ buffer(0) ]],
		 constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	struct VsOutput out;

	const struct SkyVertex vtx = ((constant struct SkyVertex *)(args->buffers[drawInfo->vertexBuffer] + drawInfo->vertexOffset))[vertexId];

	float4 pos = float4(vtx.x, vtx.y, vtx.z, 1.0);
	out.uv = pos.xyz;
	out.position = drawInfo->mvp * pos;

	return out;
}

fragment float4
Sky_FS(struct VsOutput in [[stage_in]],
		 constant struct ShaderArguments *args [[ buffer(0) ]],
		 constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	float3 v = normalize(in.uv);

	float2 uv = float2(atan2(v.z, v.x), asin(v.y));
	uv *= float2(0.1591, 0.3183);
	uv += 0.5;

	float4 color = sRGBtoLinear(args->textures[drawInfo->texture].sample(args->samplers[0], uv), drawInfo->gamma);
	//float4 color = sRGBtoLinear(args->cubeTextures[drawInfo->texture].sample(args->samplers[0], in.uv), drawInfo->gamma);

	color.rgb = tonemap(color.rgb, drawInfo->exposure, drawInfo->invGamma);
	return color;
}
