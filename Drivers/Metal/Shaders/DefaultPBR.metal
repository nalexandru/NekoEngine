#include <metal_stdlib>
#include <simd/simd.h>

#include "PBR.h"
#include "ShaderTypes.h"
#include "Tonemap.h"
#include "Material.h"

using namespace metal;

struct VsOutput
{
	float4 position [[position]];
	float4 color;
	float3 vPos;
	float2 uv;
};

struct VsOutputT
{
	float4 position [[position]];
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
			  constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	struct VsOutput out;
	struct Vertex vtx = ((constant struct Vertex *)(args->buffers[drawInfo->vertexBuffer] + drawInfo->vertexOffset))[vertexId];
	constant ModelInstance *inst = (constant ModelInstance *)(args->buffers[drawInfo->instanceBuffer] + drawInfo->instanceOffset);

	const float4 v = float4(vtx.x, vtx.y, vtx.z, 1.0);
	const float4 pos = inst->model * v;

	out.color = float4(1.0, 1.0, 1.0, 1.0);
	out.uv = float2(vtx.u, vtx.v);
	out.position = inst->mvp * v;
	out.vPos = pos.xyz / pos.w;

	return out;
}

vertex struct VsOutputT
DefaultPBR_T_VS(uint vertexId [[vertex_id]],
			  constant struct ShaderArguments *args [[ buffer(0) ]],
			  constant struct DrawInfo *drawInfo [[ buffer(1) ]])
{
	struct VsOutputT out;

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
	out.color = float4(1.0, 1.0, 1.0, 1.0);

	out.vPos = pos.xyz / pos.w;

	return out;
}

// Fragment

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

fragment float4
DefaultPBR_MR_O_FS(struct VsOutput in [[stage_in]],
				 constant struct ShaderArguments *args [[ buffer(0) ]],
				 constant struct DrawInfo *drawInfo [[ buffer(1) ]],
				 float4 wsNormal [[ color(1) ]])
{
	constant struct Material *mat = (constant struct Material *)(args->buffers[drawInfo->materialBuffer] + drawInfo->materialOffset);
	constant struct Scene *scn = (constant struct Scene *)(args->buffers[drawInfo->sceneBuffer] + drawInfo->sceneOffset);

	const float4 color = PBR_MR(args, scn, mat, in.color, in.vPos, wsNormal.xyz, in.uv);
	return float4(tonemap(color.rgb, scn->exposure, scn->invGamma), color.a);
}

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

	const float4 color = PBR_MR(args, scn, mat, in.color, in.vPos, normalize(normal), in.uv);
	return float4(tonemap(color.rgb, scn->exposure, scn->invGamma), color.a);
}

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
