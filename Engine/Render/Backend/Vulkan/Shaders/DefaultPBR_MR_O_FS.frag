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

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput in_wsNormals;

void
main()
{
	const vec4 color = PBR_MR(v_color, v_pos, subpassLoad(in_wsNormals).xyz, v_uv);
	o_fragColor = vec4(tonemap(color.rgb, DrawInfo.scene.exposure, DrawInfo.scene.invGamma), color.a);
}
