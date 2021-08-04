#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "Texture.glsl"
#include "DrawInfo.glsl"
#include "Material.glsl"
#include "Tonemap.glsl"
#include "PBR.glsl"

layout(location = 0) out vec4 o_fragColor;

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in vec3 v_pos;

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput in_wsNormals;

float
attenuate(float range, float dist)
{
	if (range <= 0.0)
		return 1.0;
	
	return max(min(1.0 - pow(dist / range, 4.0), 1.0), 0.0) / pow(dist, 2.0);
}

float
attenuateSpot(vec3 ldir, vec3 sdir, float outer, float inner)
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

void
main()
{
	Material mat = DrawInfo.material.data;
	vec3 f0 = vec3(0.04);

	// Init PBR
	vec4 albedo = /*Re_SampleSceneTexture(mat.diffuseMap, v_uv) */ mat.diffuseColor * v_color;
	if (mat.diffuseMap != 0)
		albedo *= sRGBtoLinear(Re_SampleSceneTexture(mat.diffuseMap, v_uv));

	//if (albedo.a < mat.alphaCutoff)
	//	discard;

	vec2 mr = vec2(1.0, 1.0);
	const vec3 emissive = Re_SampleSceneTexture(mat.emissionMap, v_uv).rgb * mat.emissionColor;
	float transmission = mat.transmission;

	if (mat.metallicRoughnessMap != 0)
		mr = Re_SampleSceneTexture(mat.metallicRoughnessMap, v_uv).bg;

	if (mat.transmissionMap != 0)
		transmission *= Re_SampleSceneTexture(mat.transmissionMap, v_uv).r;

	const float metallic = clamp(mr.x * mat.metallic, 0.0, 1.0);
	const float perceptualRoughness = clamp(mr.y * mat.roughness, 0.04, 1.0);

	vec3 diffuseColor = albedo.rgb * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;

	const float alphaRoughness = perceptualRoughness * perceptualRoughness;
	const float alphaRoughnessSq = alphaRoughness *  alphaRoughness;

	f0 = mix(f0, albedo.rgb, metallic);

	const float r0 = max(max(f0.r, f0.g), f0.b);
	const float r90 = clamp(r0 * 25.0, 0.0, 1.0);
	const vec3 f90 = vec3(1.0) * r90;

	const vec3 n = subpassLoad(in_wsNormals).xyz;
	const vec3 v = normalize(DrawInfo.scene.cameraPosition.xyz - v_pos);

	const float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);

	vec3 diffuse = vec3(0.0), specular = vec3(0.0);
	for (int i = 0; i < DrawInfo.scene.lightCount; ++i) {
		const Light light = DrawInfo.scene.lights[i];

		vec3 l;
		float a = 1.0;

		if (light.type == LT_POINT) {
			l = light.position.xyz - v_pos;

			const float d = length(l);
			a = 1.0;//attenuate(light.outerRadius, d);
		} else if (light.type == LT_SPOT) {
			l = light.position.xyz - v_pos;

			const float d = length(l);
			const float ra = attenuate(100.0, d);
			const float sa = attenuateSpot(l, light.direction, light.outerCutoff, light.innerCutoff);
			a = ra * sa;
		} else if (light.type == LT_DIRECTIONAL) {
			l = light.direction.xyz;
		}

		l = normalize(l);
		const vec3 radiance = light.color.xyz * vec3(light.intensity * a);
		const vec3 h = normalize(l + v);

		const float NdotL = clamp(dot(n, l), 0.001, 1.0);
		const float NdotH = clamp(dot(n, h), 0.0, 1.0);
		const float LdotH = clamp(dot(l, h), 0.0, 1.0);
		const float VdotH = clamp(dot(v, h), 0.0, 1.0);

		const vec3 F = F_Schlick(f0, f90, VdotH);
		const float V = V_SchlickSmithGGX(NdotL, NdotV, alphaRoughnessSq);
		const float D = D_GGX(NdotH, alphaRoughnessSq);

		const vec3 lightContrib = NdotL * radiance;

		diffuse += lightContrib * BRDF_Lambert(F, diffuseColor);
		specular +=  lightContrib * BRDF_Specular(F, V, D, mat.specularWeight);
	}

	vec3 color = diffuse + specular + emissive;

	o_fragColor = vec4(tonemap(color), albedo.a);
}
