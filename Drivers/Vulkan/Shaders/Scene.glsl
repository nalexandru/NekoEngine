#ifndef _RE_SCENE_H_
#define _RE_SCENE_H_

#include "Types.glsl"
#include "Light.glsl"
#include "Material.glsl"

layout(std430, buffer_reference, buffer_reference_align = 16) readonly buffer InstanceBuffer
{
	mat4 mvp;
	mat4 model;
	mat4 normal;
	VertexBuffer vertices;
	MaterialBuffer material;
};

layout(std430, buffer_reference, buffer_reference_align = 16) readonly buffer SceneBuffer
{
	mat4 viewProjection;
	vec4 cameraPosition;

	vec4 sunPosition;
	uint enviornmentMap;

	uint __padding[3];

	uint lightCount;
	uint xTileCount;

	InstanceBuffer instances;

	float exposure;
	float gamma;
	float invGamma;
	uint sampleCount;

	Light lights[];
};

#endif /* _RE_SCENE_H_ */
