#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "Texture.glsl"

layout(location = 0) in vec2 v_uv;
layout(location = 0) out float o_FragColor;

layout(push_constant) uniform ConstantDrawInfo
{
	uint ssaoTexture;
	float texelSizeX;
	float texelSizeY;
} DrawInfo;

const float offsets[4] = { -2.0, -1.0, 0.0, 1.0 };

void
main()
{
	float color = 0.0;
	vec2 offset = vec2(0.0);
	vec2 texelSize = vec2(DrawInfo.texelSizeX, DrawInfo.texelSizeY);
	uint ssaoTexture = DrawInfo.ssaoTexture;

	offset = vec2(offsets[0], offsets[0]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[0], offsets[1]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[0], offsets[2]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[0], offsets[3]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;


	offset = vec2(offsets[1], offsets[0]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[1], offsets[1]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[1], offsets[2]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[1], offsets[3]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;


	offset = vec2(offsets[2], offsets[0]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[2], offsets[1]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[2], offsets[2]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[2], offsets[3]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;


	offset = vec2(offsets[3], offsets[0]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[3], offsets[1]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[3], offsets[2]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	offset = vec2(offsets[3], offsets[3]) * texelSize;
	color += Re_SampleSceneTexture(ssaoTexture, v_uv + offset).r;

	o_FragColor = color / 16.0;
}
