#ifndef _RESOURCES_H_
#define _RESOURCES_H_

#include <Render/Material.h>

struct Material
{
	float4 diffuse;
	float4 emissive;
	float roughness;
	float metallic;
	uint textures[RE_MAX_TEXTURES];
};

// Scene Data
cbuffer RenderData : register(b0)
{
	float4x4 RD_inverseView;
	float4x4 RD_inverseProjection;
	float4x4 RD_viewProjection;
	float4x4 RD_inverseViewProjection;

/*	float3 RD_SunDirection;
	float RD_CosSunAngularRadius;
	float3 RD_SunIrradiance;
	float RD_SinSunAngularRadius;
	float3 RD_SunRenderColor;*/

	float RD_aspectRatio;
	float RD_aperture;
	uint RD_numSamples;
	uint RD_seed;

	uint RD_environmentMap;
};

// Resources
StructuredBuffer<Material> Res_Materials : register(t1, space0);
Texture2D Res_Textures[] : register(t0, space3);
SamplerState Res_Sampler : register(s0);

#endif /* _RESOURCES_H_ */