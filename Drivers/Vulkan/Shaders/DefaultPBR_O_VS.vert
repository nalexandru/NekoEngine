#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "DrawInfo.glsl"
#include "Vertex.glsl"

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_uv;
layout(location = 2) out vec3 v_pos;

void
main()
{
	vec4 v = vec4(Re_Position(), 1.0);
	v_uv = Re_TexCoord();

	const mat4 mvp = DrawInfo.instance.mvp;
	const vec4 pos = DrawInfo.instance.model * v;
	v_pos = pos.xyz / pos.w;
	v_color = vec4(1.0);

	gl_Position = mvp * v;
}
