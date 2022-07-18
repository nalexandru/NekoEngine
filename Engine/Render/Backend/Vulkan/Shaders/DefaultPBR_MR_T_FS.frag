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

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in vec4 v_color;
layout(location = 3) in vec3 v_t;
layout(location = 4) in vec3 v_b;
layout(location = 5) in vec3 v_n;

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput in_wsNormals;

void
main()
{
	vec3 normal = vec3(0.0);
	if (DrawInfo.material.data.normalMap != 0) {
		vec3 texnm = Re_SampleSceneTexture(DrawInfo.material.data.normalMap, v_uv).rgb;
		texnm = normalize(texnm * vec3(2.0) - vec3(1.0));

		const vec3 q1 = dFdx(v_pos);
		const vec3 q2 = dFdy(v_pos);
		const vec2 st1 = dFdx(v_uv);
		const vec2 st2 = dFdy(v_uv);

		const vec3 n = normalize(v_n);
		const vec3 t = normalize(q1 * st2.t - q2 * st1.t);
		const vec3 b = -normalize(cross(n, t));
		const mat3 tbn = mat3(t, b, n);

		normal = tbn * texnm;
	} else {
		normal = v_n;
	}

	const vec4 color = PBR_MR(v_color, v_pos, normalize(normal), v_uv);
	o_fragColor = vec4(tonemap(color.rgb, DrawInfo.scene.exposure, DrawInfo.scene.invGamma), color.a);
}
