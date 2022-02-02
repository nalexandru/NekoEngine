#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "Texture.glsl"

layout(location = 0) out vec4 o_fragColor;

layout(location = 0) in vec3 v_uv;

layout(std430, buffer_reference) readonly buffer VertexBuffer
{
	uint data[];
};

layout(push_constant) uniform SkyDrawInfo
{
	uint texture;
	float exposure;
	float gamma;
	float invGamma;
	mat4 mvp;
	VertexBuffer vertices;
} DrawInfo;

#include "Tonemap.glsl"

void
main()
{
	vec3 v = normalize(v_uv);
	
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= vec2(0.1591, 0.3183);
	uv += 0.5;

	vec4 color = sRGBtoLinear(Re_SampleSceneTexture(DrawInfo.texture, uv), DrawInfo.gamma);
	//vec4 color = sRGBtoLinear(Re_SampleCubeTexture(DrawInfo.texture, v_uv), DrawInfo.gamma);
	o_fragColor = vec4(tonemap(color.rgb, DrawInfo.exposure, DrawInfo.invGamma), color.a);
}
