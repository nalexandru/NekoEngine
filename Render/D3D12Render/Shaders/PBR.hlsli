#ifndef _PBR_H_
#define _PBR_H_

#include "RTUtils.hlsli"
#include "Tonemap.hlsli"
#include "Resources.hlsli"

struct ShadingInfo
{
	float4 baseColor;


	float3 f0;
	float metallic;

	float3 f90;
	float roughness;

	float alphaRoughness;
	float reflectance;
};

ShadingInfo
BuildShadingInfo(Material mtl, float2 texCoords)
{
	ShadingInfo si;

	float4 diffuseMap = sRGBToLinear(SampleTexture(Res_Textures[mtl.textures[MAP_DIFFUSE]], Res_Sampler, texCoords));
	float2 mrMap = SampleTexture(Res_Textures[mtl.textures[MAP_METALLIC_ROUGHNESS]], Res_Sampler, texCoords).xy;

	float ior = 1.5;
	float f0Ior = 0.04;

	si.f0 = lerp(float3(f0Ior, f0Ior, f0Ior), si.baseColor.xyz, si.metallic);
	si.baseColor = mtl.diffuse * diffuseMap;
	si.metallic = saturate(mtl.metallic * mrMap.x);
	si.roughness = saturate(mtl.roughness * mrMap.y);
	si.alphaRoughness = si.roughness * si.roughness;

	si.reflectance = max(max(si.f0.r, si.f0.g), si.f0.b);
	float f90refl = saturate(si.reflectance * 50.0);
	si.f90 = float3(f90refl, f90refl, f90refl);

	return si;
}

float3
F_Schlick(float3 f0, float3 f90, float VdH)
{
	return f0 + (f90 - f0) * pow(saturate(1.0 - VdH), 5.0);
}

float
V_GGX(float NdL, float NdV, float arsq)
{
	float ggxv = NdL * sqrt(NdV * NdV * (1.0 - arsq) + arsq);
	float ggxl = NdV * sqrt(NdL * NdL * (1.0 - arsq) + arsq);

	float ggx = ggxv + ggxl;

	if (ggx > 0.0)
		return 0.5 / ggx;
	else
		return 0.0;
}

float
D_GGX(float NdH, float arsq)
{
    float f = (NdH * NdH) * (arsq - 1.0) + 1.0;
    return arsq / (M_PI * f * f);
}

float3
BRDF_Lambertian(float3 f0, float3 f90, float3 diffuse, float VdH)
{
	return (float3(1.0, 1.0, 1.0) - F_Schlick(f0, f90, VdH)) * (diffuse / M_PI);
}

float4
EvaluateShading(ShadingInfo si)
{
	return si.baseColor;
}

#endif /* _PBR_H_ */