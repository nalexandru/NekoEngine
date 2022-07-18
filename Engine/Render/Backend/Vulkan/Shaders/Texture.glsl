#ifndef _RE_TEXTURES_H_
#define _RE_TEXTURES_H_

#define SCENE_SAMPLER	0
#define UI_SAMPLER		1

layout(set = 0, binding = 0) uniform sampler Re_samplers[];
layout(set = 0, binding = 1) uniform texture2D Re_textures[];
layout(set = 0, binding = 1) uniform textureCube Re_cubeTextures[];

vec4
Re_SampleSceneTexture(uint id, vec2 uv)
{
	return texture(sampler2D(Re_textures[id], Re_samplers[SCENE_SAMPLER]), uv);
}

vec4
Re_SampleUITexture(uint id, vec2 uv)
{
	return texture(sampler2D(Re_textures[id], Re_samplers[UI_SAMPLER]), uv);
}

vec4
Re_SampleCubeTexture(uint id, vec3 uv)
{
	return texture(samplerCube(Re_cubeTextures[id], Re_samplers[SCENE_SAMPLER]), uv);
}

vec4
Re_TexelFetch(uint id, ivec2 uv, int lod)
{
	return texelFetch(sampler2D(Re_textures[id], Re_samplers[SCENE_SAMPLER]), uv, lod);
}

#endif /* _RE_TEXTURES_H_ */
