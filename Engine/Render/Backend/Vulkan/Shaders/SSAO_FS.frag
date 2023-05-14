#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "Texture.glsl"
#include "Scene.glsl"

layout(location = 0) in vec2 v_uv;
layout(location = 0) out float o_FragColor;
layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput in_wsNormals;

layout(std430, buffer_reference) readonly buffer DataBuffer
{
	uint kernelSize;
	float radius;
	float powerExponent;
	float threshold;
	float bias;
	uint noiseSize;
	uint noiseTexture;
	float padding;
	vec4 kernel[];
};

layout(push_constant) uniform ConstantDrawInfo
{
	mat4 inverseView;
	DataBuffer data;
	SceneBuffer scene;
	uint depthTexture;
} DrawInfo;

vec3
posFromDepth(vec2 uv)
{
	vec4 projPos = vec4(uv.x * 2.0 - 1.0, (1.0 - uv.y) * 2.0 - 1.0, 1.0 - Re_SampleSceneTexture(DrawInfo.depthTexture, v_uv).r, 1.0);
	vec4 pos = DrawInfo.scene.inverseProjection * projPos;
	return (pos.xyz / pos.w);
}

void
main()
{
	vec3 fragPos = posFromDepth(v_uv);
	vec3 normal = normalize((vec4(subpassLoad(in_wsNormals).xyz, 1.0) * DrawInfo.inverseView).xyz);
	vec3 rand = vec3(Re_SampleSceneTexture(DrawInfo.data.noiseTexture, v_uv * DrawInfo.data.noiseSize).xy, 0.0);
	vec3 tangent = normalize(rand - normal * dot(rand, normal));
	vec3 bitangent = cross(tangent, normal);
	mat3 tbn = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < int(DrawInfo.data.kernelSize); ++i) {
		if (dot(DrawInfo.data.kernel[i].xyz, normal) < DrawInfo.data.threshold)
			continue;

		vec3 samplePos = tbn * DrawInfo.data.kernel[i].xyz;
		samplePos = fragPos + samplePos * DrawInfo.data.radius;

		vec4 offset = vec4(samplePos, 1.0);
		offset = DrawInfo.scene.projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = posFromDepth(offset.xy).z;
		sampleDepth = offset.z;
		float rangeCheck = smoothstep(0.0, 1.0, DrawInfo.data.radius / abs(fragPos.z - sampleDepth));

		occlusion += (sampleDepth <= samplePos.z + DrawInfo.data.bias ? 1.0 : 0.0) * rangeCheck;
	}

	o_FragColor = pow(1.0 - (occlusion / DrawInfo.data.kernelSize), DrawInfo.data.powerExponent);
}
