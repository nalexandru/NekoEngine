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

struct ShaderArguments
{
	const array<sampler, 3> samplers [[ id(0) ]];
	const array<texture2d<float>, 65535> textures [[ id(3) ]];
	const array<constant uint8_t *, 65535> buffers [[ id(65538) ]];
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
DefaultPBR_VS(uint vertexId [[vertex_id]],
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

// Fragment

constant float3 F0 = float3(0.04);

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
DefaultPBR_MR_FS(struct VsOutput in [[stage_in]],
				 constant struct ShaderArguments *args [[ buffer(0) ]],
				 constant struct DrawInfo *drawInfo [[ buffer(1) ]],
				 float4 wsNormal [[ color(1) ]])
{
	const struct Material mat = *(constant struct Material *)(args->buffers[drawInfo->materialBuffer] + drawInfo->materialOffset);
	constant struct Scene *scn = (constant struct Scene *)(args->buffers[drawInfo->sceneBuffer] + drawInfo->sceneOffset);

	float4 albedo = mat.diffuseColor * in.color;
	if (mat.diffuseMap)
		albedo *= sRGBtoLinear(args->textures[mat.diffuseMap].sample(args->samplers[0], in.uv), scn->gamma);

	const float3 emissive = args->textures[mat.emissionMap].sample(args->samplers[0], in.uv).xyz * mat.emissionColor.rgb;
	//float transmission = mat.transmission;

	float2 mr = float2(1.0, 1.0);
	if (mat.metallicRoughnessMap)
		mr = args->textures[mat.metallicRoughnessMap].sample(args->samplers[0], in.uv).bg;

	const float metallic = clamp(mr.x * mat.emissionColor.a, 0.0, 1.0);
	const float perceptualRoughness = clamp(mr.y * mat.roughness, 0.04, 1.0);

	float3 diffuseColor = albedo.rgb * (float3(1.0) - F0);
	diffuseColor *= 1.0 - metallic;

	const float alphaRoughness = perceptualRoughness * perceptualRoughness;
	const float alphaRoughnessSq = alphaRoughness * alphaRoughness;

	float3 f0 = mix(F0, albedo.rgb, metallic);

	const float r0 = max(max(f0.r, f0.g), f0.b);
	const float r90 = clamp(r0 * 25.0, 0.0, 1.0);
	const float3 f90 = float3(r90);

	const float3 n = wsNormal.rgb;
	const float3 v = normalize(scn->cameraPosition.xyz - in.vPos);

	const float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);

	float3 diffuse = float3(0.0), specular = float3(0.0);
	constant struct Light *lights = (constant struct Light *)&scn->lightStart;
	for (uint i = 0; i < scn->lightCount; ++i) {
		const struct Light light = lights[i];

		float3 l = float3(0.0);
		float a = 1.0;

		if (light.type == LT_POINT) {
			l = float3(light.x, light.y, light.z) - in.vPos;

			//const float d = length(l);
			a = 1.0;
		} else if (light.type == LT_SPOT) {
			l = float3(light.x, light.y, light.z) - in.vPos;

			const float d = length(l);
			const float ra = Attenuate(light.outerRadius, d);
			const float sa = AttenuateSpot(l, light.direction.xyz, light.outerCutoff, light.innerCutoff);
			a = ra * sa;
		} else if (light.type == LT_DIRECTIONAL) {
			l = light.direction.xyz;
		}

		l = normalize(l);
		const float3 radiance = light.color.rgb * float3(light.color.a * a);
		const float3 h = normalize(l + v);

		const float NdotL = clamp(dot(n, l), 0.001, 1.0);
		const float NdotH = clamp(dot(n, h), 0.0, 1.0);
		const float VdotH = clamp(dot(v, h), 0.0, 1.0);

		const float3 F = F_Schlick(f0, f90, VdotH);
		const float V = V_SchlickSmithGGX(NdotL, NdotV, alphaRoughnessSq);
		const float D = D_GGX(NdotH, alphaRoughnessSq);

		const float3 lightContrib = NdotL * radiance;

		diffuse += lightContrib * BRDF_Lambert(F, diffuseColor);
		specular +=  lightContrib * BRDF_Specular(F, V, D, mat.specularWeight);
	}

	float3 color = diffuse + specular + emissive;;

	return float4(tonemap(color, scn->exposure, scn->invGamma), albedo.a);
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

float
Attenuate(float range, float dist)
{
	if (range <= 0.0)
		return 1.0;

	return max(min(1.0 - pow(dist / range, 4.0), 1.0), 0.0) / pow(dist, 2.0);
}

float
AttenuateSpot(float3 ldir, float3 sdir, float outer, float inner)
{
	float actualCos = dot(normalize(sdir), normalize(-ldir));
	if (actualCos > outer) {
		if (actualCos < inner)
			return smoothstep(outer, inner, actualCos);
		else
			return 1.0;
	} else {
		return 0.0;
	}
}

