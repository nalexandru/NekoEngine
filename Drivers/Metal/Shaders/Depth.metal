#include <metal_stdlib>
#include <simd/simd.h>

#include "Material.h"
#include "ShaderTypes.h"

using namespace metal;

struct VsOutput
{
	float4 position [[position]];
	float2 uv;
	float3 normal;
	float3 tangent;
	float3 biTangent;
	float3 vPos;
};

struct ShaderArguments
{
	const array<sampler, 3> samplers [[ id(0) ]];
	const array<texture2d<float>, 65535> textures [[ id(3) ]];
	const array<constant uint8_t *, 65535> buffers [[ id(65538) ]];
};

struct DrawInfo
{
	NE_BUFFER(vertex);
	NE_BUFFER(material);
	NE_BUFFER(instance);
};

vertex struct VsOutput
Depth_VS(uint vertexId [[vertex_id]],
		 constant struct ShaderArguments *args [[ buffer(0) ]],
		 constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	struct VsOutput out;

	const struct Vertex vtx = ((constant struct Vertex *)(args->buffers[drawInfo->vertexBuffer] + drawInfo->vertexOffset))[vertexId];
	constant ModelInstance *inst = (constant ModelInstance *)(args->buffers[drawInfo->instanceBuffer] + drawInfo->instanceOffset);

	const float3x3 normalMat = float3x3(inst->normal.columns[0].xyz,
										inst->normal.columns[1].xyz,
										inst->normal.columns[2].xyz);
	const float4 v = float4(vtx.x, vtx.y, vtx.z, 1.0);
	const float4 pos = inst->model * v;

	out.tangent = normalize(normalMat * float3(vtx.tx, vtx.ty, vtx.tz));
	out.normal = normalize(normalMat * float3(vtx.nx, vtx.ny, vtx.nz));
	out.biTangent = normalize(normalMat * cross(out.tangent, out.normal));
	out.uv = float2(vtx.u, vtx.v);
	out.position = inst->mvp * v;

	out.vPos = pos.xyz / pos.w;

	return out;
}

fragment float4
Depth_FS(struct VsOutput in [[stage_in]],
		 constant struct ShaderArguments *args [[ buffer(0) ]],
		 constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	constant struct Material *mat = (constant struct Material *)(args->buffers[drawInfo->materialBuffer] + drawInfo->materialOffset);

	if (!mat->normalMap)
		return float4(normalize(in.normal), 1.0);

	float3 texnm = args->textures[mat->normalMap].sample(args->samplers[0], in.uv).xyz;
	texnm = normalize(texnm * float3(2.0) - float3(1.0));

	const float3 q1 = dfdx(in.vPos);
	const float3 q2 = dfdy(in.vPos);
	const float2 st1 = dfdx(in.uv);
	const float2 st2 = dfdy(in.uv);

	const float3 n = normalize(in.normal);
	const float3 t = normalize(q1 * st2.y - q2 * st1.y);
	const float3 b = -normalize(cross(n, t));
	const float3x3 tbn = float3x3(t, b, n);

	return float4(tbn * texnm, 1.0);
}
