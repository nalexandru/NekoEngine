#ifndef _RE_PBR_H_
#define _RE_PBR_H_

#include "Light.glsl"
#include "Texture.glsl"
#include "Constants.glsl"
#include "Material.glsl"
#include "DrawInfo.glsl"

struct PBRData
{
	vec3 f0;
	float VdotH;

	vec3 f90;
	float NdotL;

	vec3 diffuseColor;
	float NdotV;

	float metallic;
	float alphaRoughnessSq;
};

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

	return vec3(0.0);
}

vec4
PBRMain(const Material mat, const vec4 color, const vec3 pos,
		const vec3 n, const vec2 uv, const float metallic, const float perceptualRoughness)
{
	vec3 f0 = vec3(0.04);

	if (mat.alphaMaskMap != 0) {
		float mask = Re_SampleSceneTexture(mat.alphaMaskMap, uv).r;
		if (mask < mat.alphaCutoff)
			discard;
	}

	// Init PBR
	vec4 albedo = mat.diffuseColor * color;
	if (mat.diffuseMap != 0)
		albedo *= sRGBtoLinear(Re_SampleSceneTexture(mat.diffuseMap, uv), DrawInfo.scene.gamma);

	if (albedo.a < 0.01)
		discard;

	const vec3 emissive = Re_SampleSceneTexture(mat.emissionMap, uv).rgb * mat.emissionColor;
	float transmission = mat.transmission;

	if (mat.transmissionMap != 0)
		transmission *= Re_SampleSceneTexture(mat.transmissionMap, uv).r;

	vec3 diffuseColor = albedo.rgb * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;

	const float alphaRoughness = perceptualRoughness * perceptualRoughness;
	const float alphaRoughnessSq = alphaRoughness *  alphaRoughness;

	f0 = mix(f0, albedo.rgb, metallic);

	const float r0 = max(max(f0.r, f0.g), f0.b);
	const float r90 = clamp(r0 * 25.0, 0.0, 1.0);
	const vec3 f90 = vec3(1.0) * r90;

	const vec3 v = normalize(DrawInfo.scene.cameraPosition.xyz - pos);

	const float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);

	const ivec2 location = ivec2(gl_FragCoord.xy);
	const ivec2 tile = location / ivec2(16, 16);
	const uint lightIndex = tile.y * int(DrawInfo.scene.xTileCount) + tile.x;

	vec3 diffuse = vec3(0.0), specular = vec3(0.0);
	const uint lightOffset = lightIndex * DrawInfo.scene.lightCount;
	for (int i = 0; i < DrawInfo.scene.lightCount && DrawInfo.visibleIndices.data[lightOffset + i] != -1; ++i) {
		const Light light = DrawInfo.scene.lights[DrawInfo.visibleIndices.data[lightOffset + i]];
		float a = 1.0;
		vec3 l;

		if (light.type == LT_POINT) {
			l = light.position.xyz - pos;
			const float d = length(l);
			l = normalize(l);

			a = attenuate(light.outerRadius, d);
		} else if (light.type == LT_SPOT) {
			l = light.position.xyz - pos;
			const float d = length(l);
			l = normalize(l);

			const float ra = attenuate(light.outerRadius, d);
			const float sa = attenuateSpot(l, light.direction, light.outerCutoff, light.innerCutoff);
			a = ra * sa;
		} else if (light.type == LT_DIRECTIONAL) {
			l = normalize(light.direction);
		}

		const vec3 radiance = light.color.xyz * vec3(light.intensity * a);
		const vec3 h = normalize(l + v);

		const float NdotL = clamp(dot(n, l), 0.001, 1.0);
		const float NdotH = clamp(dot(n, h), 0.0, 1.0);
		const float VdotH = clamp(dot(v, h), 0.0, 1.0);

		const vec3 F = F_Schlick(f0, f90, VdotH);
		const float V = V_SchlickSmithGGX(NdotL, NdotV, alphaRoughnessSq);
		const float D = D_GGX(NdotH, alphaRoughnessSq);

		const vec3 lightContrib = NdotL * radiance;

		diffuse += lightContrib * BRDF_Lambert(F, diffuseColor);
		specular += lightContrib * BRDF_Specular(F, V, D, mat.specularWeight);
	}

	vec4 finalColor = vec4(diffuse + specular + emissive, albedo.a);
	if (DrawInfo.aoMap != 0)
		finalColor *= Re_SampleSceneTexture(DrawInfo.aoMap, gl_FragCoord.xy).r;

	return finalColor;
}

vec4
PBR_MR(const vec4 color, const vec3 pos, const vec3 n, const vec2 uv)
{
	const Material mat = DrawInfo.material.data;

	vec2 mr = vec2(1.0, 1.0);
	if (mat.metallicRoughnessMap != 0)
		mr = Re_SampleSceneTexture(mat.metallicRoughnessMap, uv).bg;

	const float metallic = clamp(mr.x * mat.metallic, 0.0, 1.0);
	const float perceptualRoughness = clamp(mr.y * mat.roughness, 0.04, 1.0);

	return PBRMain(mat, color, pos, n, uv, metallic, perceptualRoughness);
}

vec4
PBR_SG(const vec4 color, const vec3 pos, const vec3 n, const vec2 uv)
{
	// TODO
	const Material mat = DrawInfo.material.data;

	vec2 mr = vec2(1.0, 1.0);
	if (mat.metallicRoughnessMap != 0)
		mr = Re_SampleSceneTexture(mat.metallicRoughnessMap, uv).bg;

	const float metallic = clamp(mr.x * mat.metallic, 0.0, 1.0);
	const float perceptualRoughness = clamp(mr.y * mat.roughness, 0.04, 1.0);

	return PBRMain(mat, color, pos, n, uv, metallic, perceptualRoughness);
}

#endif /* _RE_PBR_H_ */

/* NekoEngine
 *
 * PBR.glsl
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
