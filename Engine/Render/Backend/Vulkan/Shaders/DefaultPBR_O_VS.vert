#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "DrawInfo.glsl"
#include "Vertex.glsl"

invariant gl_Position;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;
layout(location = 3) in vec2 a_uv;
layout(location = 4) in vec4 a_color;

layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec2 v_uv;
layout(location = 2) out vec4 v_color;

void
main()
{
	vec4 v = vec4(a_pos, 1.0);
	v_uv = a_uv;

	const mat4 mvp = DrawInfo.instance.mvp;
	const vec4 pos = DrawInfo.instance.model * v;
	v_pos = pos.xyz / pos.w;
	v_color = a_color;

	gl_Position = mvp * v;
}
