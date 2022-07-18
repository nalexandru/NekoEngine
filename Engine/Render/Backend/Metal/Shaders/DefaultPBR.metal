#include <metal_stdlib>
#include <simd/simd.h>

#include "PBR.h"
#include "ShaderTypes.h"
#include "Tonemap.h"
#include "Material.h"

using namespace metal;

struct VsOutput
{
	float4 position [[position, invariant]];
	float4 color;
	float3 vPos;
	float2 uv;
};

struct VsOutputT
{
	float4 position [[position, invariant]];
	float4 color;
	float2 uv;
	float3 normal;
	float3 tangent;
	float3 biTangent;
	float3 vPos;
};

struct DrawInfo
{
	NE_BUFFER(vertex);
	NE_BUFFER(scene);
	NE_BUFFER(visibleIndicesAddress);
	NE_BUFFER(instance);
	NE_BUFFER(material);
};

float Attenuate(float range, float dist);
float AttenuateSpot(float3 ldir, float3 sdir, float outer, float inner);

// Vertex

vertex struct VsOutput
DefaultPBR_O_VS(uint vertexId [[vertex_id]],
			  constant struct ShaderArguments *args [[ buffer(0) ]],
			  constant struct DrawInfo *drawInfo [[ buffer(1) ]],
			  struct Vertex vtx [[stage_in]])
{
	struct VsOutput out;
	constant ModelInstance *inst = (constant ModelInstance *)(args->buffers[drawInfo->instanceBuffer] + drawInfo->instanceOffset);

	const float4 v = float4(vtx.position, 1.0);
	const float4 pos = inst->model * v;

	out.color = vtx.color;
	out.uv = vtx.uv;
	out.position = inst->mvp * v;
	out.vPos = pos.xyz / pos.w;

	return out;
}

vertex struct VsOutputT
DefaultPBR_T_VS(uint vertexId [[vertex_id]],
			  constant struct ShaderArguments *args [[ buffer(0) ]],
			  constant struct DrawInfo *drawInfo [[ buffer(1) ]],
			  struct Vertex vtx [[stage_in]])
{
	struct VsOutputT out;
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
	out.color = vtx.color;

	out.vPos = pos.xyz / pos.w;

	return out;
}

// Fragment

[[early_fragment_tests]]
fragment float4
Unlit_FS(struct VsOutput in [[stage_in]],
		 constant struct ShaderArguments *args [[ buffer(0) ]],
		 constant struct DrawInfo *drawInfo [[ buffer(1) ]],
		 float4 wsNormal [[ color(1) ]])
{
	constant struct Material *mat = (constant struct Material *)(args->buffers[drawInfo->materialBuffer] + drawInfo->materialOffset);
	constant struct Scene *scn = (constant struct Scene *)(args->buffers[drawInfo->sceneBuffer] + drawInfo->sceneOffset);

	float4 color = mat->diffuseColor * in.color;
	if (mat->diffuseMap)
		color *= sRGBtoLinear(args->textures[mat->diffuseMap].sample(args->samplers[0], in.uv), scn->gamma);
	color.rgb = tonemap(color.rgb, scn->exposure, scn->invGamma);

	return color;
}

[[early_fragment_tests]]
fragment float4
DefaultPBR_MR_O_FS(struct VsOutput in [[stage_in]],
				 constant struct ShaderArguments *args [[ buffer(0) ]],
				 constant struct DrawInfo *drawInfo [[ buffer(1) ]],
				 float4 wsNormal [[ color(1) ]])
{
	constant struct Material *mat = (constant struct Material *)(args->buffers[drawInfo->materialBuffer] + drawInfo->materialOffset);
	constant struct Scene *scn = (constant struct Scene *)(args->buffers[drawInfo->sceneBuffer] + drawInfo->sceneOffset);

	const int2 location = int2(in.position.xy);
	const int2 tile = location / int2(16, 16);
	const uint lightOffset = (tile.y * scn->xTileCount + tile.x) * scn->lightCount;
	constant int32_t *visibleIndices = (constant int32_t *)(args->buffers[drawInfo->visibleIndicesAddressBuffer] + drawInfo->visibleIndicesAddressOffset);

	const float4 color = PBR_MR(args, scn, mat, &visibleIndices[lightOffset], in.color, in.vPos, wsNormal.xyz, in.uv);
	return float4(tonemap(color.rgb, scn->exposure, scn->invGamma), color.a);
}

[[early_fragment_tests]]
fragment float4
DefaultPBR_MR_T_FS(struct VsOutputT in [[stage_in]],
				 constant struct ShaderArguments *args [[ buffer(0) ]],
				 constant struct DrawInfo *drawInfo [[ buffer(1) ]],
				 float4 wsNormal [[ color(1) ]])
{
	constant struct Material *mat = (constant struct Material *)(args->buffers[drawInfo->materialBuffer] + drawInfo->materialOffset);
	constant struct Scene *scn = (constant struct Scene *)(args->buffers[drawInfo->sceneBuffer] + drawInfo->sceneOffset);

	float3 normal = float3(0.0);
	if (mat->normalMap) {
		float3 texnm = args->textures[mat->normalMap].sample(args->samplers[0], in.uv).rgb;
		texnm = normalize(texnm * float3(2.0) - float3(1.0));

		const float3 q1 = dfdx(in.vPos);
		const float3 q2 = dfdy(in.vPos);
		const float2 st1 = dfdx(in.uv);
		const float2 st2 = dfdy(in.uv);

		const float3 n = normalize(in.normal);
		const float3 t = normalize(q1 * st2.y - q2 * st1.y);
		const float3 b = -normalize(cross(n, t));
		const float3x3 tbn = float3x3(t, b, n);

		normal = tbn * texnm;
	} else {
		normal = in.normal;
	}

	const int2 location = int2(in.position.xy);
	const int2 tile = location / int2(16, 16);
	const uint lightOffset = (tile.y * scn->xTileCount + tile.x) * scn->lightCount;
	constant int32_t *visibleIndices = (constant int32_t *)(args->buffers[drawInfo->visibleIndicesAddressBuffer] + drawInfo->visibleIndicesAddressOffset);

	const float4 color = PBR_MR(args, scn, mat, &visibleIndices[lightOffset], in.color, in.vPos, normalize(normal), in.uv);
	return float4(tonemap(color.rgb, scn->exposure, scn->invGamma), color.a);
}

[[early_fragment_tests]]
fragment float4
DefaultPBR_SG_FS(struct VsOutput in [[stage_in]])
{
	return in.color;
}

/*float4
AccumulateLights(MaterialInfo mi, float3 vPos, constant struct Scene *scn)
{
	/constant struct Light *lights = (constant struct Light *)(scn + sizeof(struct Scene));

	float4 color = float4(0.0);
	for (uint i = 0; i < scn->lightCount; ++i) {
		struct Light l = lights[i];
		float3 dir = float3(0.0), view = float3(0.0), light = float3(0.0);

		if (l.type == LT_DIRECTIONAL) {
			dir = l.direction;
			light = l.intensity * l.color;
		} else if (l.type == LT_SPOT) {
			dir = l.position - vPos;

			const float d = length(dir);
			const float ratten = Attenuate(l.outerRadius, d);
			const float satten = AttenuateSpot(dir, l.direction, l.outerCutoff, l.innerCutoff);
			const float intensity = ratten * satten * l.intensity;

			light =  intensity * l.color;
		} else if (l.type == LT_POINT) {
			dir = l.position - vPos;

			const float d = length(dir);
			const float intensity = l.intensity * max(min(1.0 - pow(d / l.outerRadius, 4.0), 1.0), 0.0) / pow(d, 2.0);

			light = intensity * l.color;
		}

		color += float4(light, 1.0) * Re_EvaluateMaterial(mi, dir, view);
	}

	color = mi.diffuseColor;
	return color;*
	return float4(0.0);
}*/
