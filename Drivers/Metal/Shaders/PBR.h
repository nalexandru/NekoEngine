#ifndef PBR_h
#define PBR_h

#include <metal_stdlib>
using namespace metal;

#include "Light.h"
#include "Material.h"
#include "Tonemap.h"
#include "ShaderTypes.h"

#define PI 3.141592653589793

struct PBRArgs
{
	constant struct ShaderArguments *shaderArgs;
	float4 vertexColor;
	float3 viewPosition;
	float3 normal;
	float2 uv;
};

inline float3
F_Schlick(float3 f0, float3 f90, float VdotH)
{
	return f0 + (f90 - f0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

inline float
V_SchlickSmithGGX(float NdotL, float NdotV, float arsq)
{
	const float ggxv = NdotL * sqrt(NdotV * (NdotV - NdotV * arsq) + arsq);
	const float ggxl = NdotV * sqrt(NdotL * (NdotL - NdotL * arsq) + arsq);
	return 0.5 / (ggxv + ggxl);
}

inline float
D_GGX(float NdotH, float arsq)
{
	const float f = (NdotH * arsq - NdotH) * NdotH + 1.0;
	return arsq / (PI * f * f);
}

inline float3
BRDF_Lambert(float3 F, float3 diffuseColor)
{
	return (1.0 - F) * (diffuseColor / PI);
}

inline float3
BRDF_Specular(float3 F, float V, float D, float weight)
{
	return float3(weight) * F * float3(V) * float3(D);
}

inline float3
IBL_Radiance(float3 n, float3 v, float NdotV, float r, float3 f0, float3 color, float weight)
{
/*	const float lod = r * float(u_MipCount - 1);
	const vec3 reflection = normalize(reflect(-v, n));
	const vec2 samplePoint = clamp(vec2(NdotV, roughness), vec2(0.0, 0.0), vec2(1.0, 1.0));
	const vec2 fAB = texture(u_GGXLUT, samplePoint).rg;

	vec4 specularSample = getSpecularSample(reflection, lod);
	vec3 irradiance = getDiffuseLight(n);

	const vec3 fr = max(vec3(1.0 - r), f0) - f0;
	const vec3 s = f0 + fr * pow(1.0 - NdotV, 5.0);
	const vec3 fssess = weight * s * fAB.x + fAB.y;

	const float ems = (1.0 - (fAB.x + fAB.y));
	const vec3 fAVG = weight * (f0 + (1.0 - f0) / 21.0);
	const vec3 fmsems = ems * fssess * fAVG / (1.0 - fAVG * ems);

	const vec3 specular = fssess * specularSample.xyz;
	const vec3 diffuse = (fmsems + (color * (1.0 - fssess + fmsems)) * irradiance;*/

	return float3(0.0);
}

inline float4
PBRMain(constant struct ShaderArguments *sa, constant struct Scene *scn, constant struct Material *mat,
		float4 vertexColor, float3 viewPosition, float3 n, float2 uv, float metallic, float perceptualRoughness)
{
	float3 f0 = float3(0.04);

	if (mat->alphaMaskMap) {
		const float mask = sa->textures[mat->alphaMaskMap].sample(sa->samplers[0], uv).r;
		if (mask < mat->alphaCutoff)
			discard_fragment();
	}

	float4 albedo = mat->diffuseColor * vertexColor;
	if (mat->diffuseMap)
		albedo *= sRGBtoLinear(sa->textures[mat->diffuseMap].sample(sa->samplers[0], uv), scn->gamma);

	if (albedo.a < 0.01)
		discard_fragment();

	const float3 emissive = sa->textures[mat->emissionMap].sample(sa->samplers[0], uv).xyz * mat->emissionColor.rgb;

	float3 diffuseColor = albedo.rgb * (float3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;

	const float alphaRoughness = perceptualRoughness * perceptualRoughness;
	const float alphaRoughnessSq = alphaRoughness * alphaRoughness;

	f0 = mix(f0, albedo.rgb, metallic);

	const float r0 = max(max(f0.r, f0.g), f0.b);
	const float r90 = clamp(r0 * 25.0, 0.0, 1.0);
	const float3 f90 = float3(r90);


	const float3 v = normalize(scn->cameraPosition.xyz - viewPosition);

	const float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);

	float3 diffuse = float3(0.0), specular = float3(0.0);
	constant struct Light *lights = (constant struct Light *)&scn->lightStart;
	for (uint i = 0; i < scn->lightCount; ++i) {
		const struct Light light = lights[i];

		float3 l = float3(0.0);
		float a = 1.0;

		if (light.type == LT_POINT) {
			l = float3(light.x, light.y, light.z) - viewPosition;
			const float d = length(l);
			l = normalize(l);

			a = attenuate(light.outerRadius, d);
		} else if (light.type == LT_SPOT) {
			l = float3(light.x, light.y, light.z) - viewPosition;
			const float d = length(l);
			l = normalize(l);

			const float ra = attenuate(light.outerRadius, d);
			const float sa = attenuateSpot(l, light.direction.xyz, light.outerCutoff, light.innerCutoff);
			a = ra * sa;
		} else if (light.type == LT_DIRECTIONAL) {
			l = normalize(light.direction.xyz);
		}

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
		specular +=  lightContrib * BRDF_Specular(F, V, D, mat->specularWeight);
	}

	return float4(diffuse + specular + emissive, albedo.a);
}

inline float4
PBR_MR(constant struct ShaderArguments *sa, constant struct Scene *scn, constant struct Material *mat,
	   float4 vertexColor, float3 viewPosition, float3 normal, float2 uv)
{
	float2 mr = float2(1.0, 1.0);
	if (mat->metallicRoughnessMap)
		mr = sa->textures[mat->metallicRoughnessMap].sample(sa->samplers[0], uv).bg;

	const float metallic = clamp(mr.x * mat->emissionColor.a, 0.0, 1.0);
	const float perceptualRoughness = clamp(mr.y * mat->roughness, 0.04, 1.0);

	return PBRMain(sa, scn, mat, vertexColor, viewPosition, normal, uv, metallic, perceptualRoughness);
}

#endif /* PBR_h */
