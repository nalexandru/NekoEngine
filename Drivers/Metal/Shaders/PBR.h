#ifndef PBR_h
#define PBR_h

#include <metal_stdlib>
using namespace metal;

#define PI 3.141592653589793

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
	return float3(0.0);
}

#endif /* PBR_h */
