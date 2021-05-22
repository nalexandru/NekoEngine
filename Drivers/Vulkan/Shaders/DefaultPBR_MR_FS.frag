#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "Texture.h"
#include "DrawInfo.h"

layout(location = 0) out vec4 o_fragColor;

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_uv;

void
main()
{
	o_fragColor = Re_SampleSceneTexture(DrawInfo.material.data.diffuseMap, v_uv);
}
