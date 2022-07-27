#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "Texture.glsl"
#include "DepthDrawInfo.glsl"

layout(location = 0) out vec3 o_wsNormals;

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in vec3 v_normal;

layout(early_fragment_tests) in;

void
main()
{
//	if (DrawInfo.material.data.alphaMaskMap != 0) {
//		float mask = Re_SampleSceneTexture(DrawInfo.material.data.alphaMaskMap, v_uv).r;
//		if (mask < DrawInfo.material.data.alphaCutoff)
//			discard;
//	}

	if (DrawInfo.material.data.normalMap == 0) {
		o_wsNormals = normalize(v_normal);
		return;
	}

	vec3 texnm = Re_SampleSceneTexture(DrawInfo.material.data.normalMap, v_uv).rgb;
	texnm = normalize(texnm * vec3(2.0) - vec3(1.0));

	const vec3 q1 = dFdx(v_pos);
	const vec3 q2 = dFdy(v_pos);
	const vec2 st1 = dFdx(v_uv);
	const vec2 st2 = dFdy(v_uv);

	const vec3 n = normalize(v_normal);
	const vec3 t = normalize(q1 * st2.t - q2 * st1.t);
	const vec3 b = normalize(cross(t, n));
	const mat3 tbn = mat3(t, b, n);

	// FIXME
	/*
	const vec3 n = normalize(v_normal);
	const vec3 t = normalize(v_tangent);
	const vec3 b = normalize(cross(t, n));
	const mat3 tbn = mat3(t, b, n);
	*/

	o_wsNormals = normalize(tbn * texnm);
}
