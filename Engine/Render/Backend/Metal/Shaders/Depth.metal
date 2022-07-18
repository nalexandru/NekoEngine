#include <metal_stdlib>
#include <simd/simd.h>

#include "Material.h"
#include "ShaderTypes.h"

using namespace metal;

struct VsOutput
{
	float4 position [[position, invariant]];
	float2 uv;
	float3 normal;
	float3 tangent;
	float3 biTangent;
	float3 vPos;
};

struct DrawInfo
{
	NE_BUFFER(vertex);
	NE_BUFFER(material);
	NE_BUFFER(instance);
};

struct VertexS
{
	float3 position [[attribute(0)]];
	float3 normal [[attribute(1)]];
	float3 tangent [[attribute(2)]];
	float2 uv [[attribute(3)]];
	float4 color [[attribute(4)]];
};

vertex struct VsOutput
Depth_VS(uint vertexId [[vertex_id]],
		 constant struct ShaderArguments *args [[ buffer(0) ]],
		 constant struct DrawInfo *drawInfo [[ buffer(1) ]],
		 struct Vertex vtx [[stage_in]])
{
	struct VsOutput out;

	constant ModelInstance *inst = (constant ModelInstance *)(args->buffers[drawInfo->instanceBuffer] + drawInfo->instanceOffset);

	const float3x3 normalMat = float3x3(inst->normal.columns[0].xyz,
										inst->normal.columns[1].xyz,
										inst->normal.columns[2].xyz);
	const float4 v = float4(vtx.position, 1.0);
	const float4 pos = inst->model * v;

	out.tangent = normalize(normalMat * vtx.tangent);
	out.normal = normalize(normalMat * vtx.normal);
	out.biTangent = normalize(normalMat * cross(out.tangent, out.normal));
	out.uv = vtx.uv;
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
