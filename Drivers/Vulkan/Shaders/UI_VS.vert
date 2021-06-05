#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "UIDrawInfo.h"

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_uv;

void
main()
{
	vec2 pos = vec2(DrawInfo.vertices.data[gl_VertexIndex].x, DrawInfo.vertices.data[gl_VertexIndex].y);
	v_uv = vec2(DrawInfo.vertices.data[gl_VertexIndex].u, DrawInfo.vertices.data[gl_VertexIndex].v);
	v_color = DrawInfo.vertices.data[gl_VertexIndex].color;
	gl_Position = DrawInfo.mvp * vec4(pos, 0.0, 1.0);
}
