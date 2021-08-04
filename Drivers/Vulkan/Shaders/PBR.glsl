#ifndef _RE_PBR_H_
#define _RE_PBR_H_

#include "Texture.glsl"
#include "Constants.glsl"
#include "Material.glsl"

vec3
F_Schlick(vec3 f0, vec3 f90, float VdotH)
{
	return f0 + (f90 - f0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

float
V_SchlickSmithGGX(float NdotL, float NdotV, float arsq)
{
	const float ggxv = NdotL * sqrt(NdotV * (NdotV - NdotV * arsq) + arsq);
	const float ggxl = NdotV * sqrt(NdotL * (NdotL - NdotL * arsq) + arsq);
	return 0.5 / (ggxv + ggxl);
}

float
D_GGX(float NdotH, float arsq)
{
	const float f = (NdotH * arsq - NdotH) * NdotH + 1.0;
	return arsq / (PI * f * f);
}

vec3
BRDF_Lambert(vec3 F, vec3 diffuseColor)
{
	return (1.0 - F) * (diffuseColor / PI);
}

vec3
BRDF_Specular(vec3 F, float V, float D, float weight)
{
	return vec3(weight) * F * vec3(V) * vec3(D);
}

vec3
IBL_Radiance(vec3 n, vec3 v, float NdotV, float r, vec3 f0, vec3 color, float weight)
{
	return vec3(0.0);
}

#endif /* _RE_PBR_H_ */
