#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "DepthDrawInfo.glsl"
#include "Vertex.glsl"

invariant gl_Position;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;
layout(location = 3) in vec2 a_uv;
layout(location = 4) in vec4 a_color;

layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec2 v_uv;
layout(location = 2) out vec3 v_t;
layout(location = 3) out vec3 v_b;
layout(location = 4) out vec3 v_n;

void
main()
{
	const vec4 v = vec4(a_pos, 1.0);
	const vec3 n = a_normal;// / vec3(127.0) - vec3(1.0);
	const vec3 t = a_tangent;// / vec3(127.0) - vec3(1.0);
	v_uv = a_uv;
	const mat4 mvp = DrawInfo.instance.mvp;
	const mat3 normalMat = mat3(DrawInfo.instance.normal);

	const vec4 pos = DrawInfo.instance.model * v;

	v_t = normalize(normalMat * t);
	v_n = normalize(normalMat * n);
	v_b = normalize(normalMat * cross(v_t, v_n));
	v_pos = pos.xyz / pos.w;

	gl_Position = mvp * v;
}
