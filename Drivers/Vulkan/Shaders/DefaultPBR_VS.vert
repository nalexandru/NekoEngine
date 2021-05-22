#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "DrawInfo.h"
#include "Vertex.h"

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_uv;

void
main()
{
	vec3 v = Re_Position();
	v_uv = Re_TexCoord();

	v_color = vec4(1.0);

	gl_Position = vec4(v, 1.0);
}
