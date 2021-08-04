#ifndef _RE_MATERIAL_H_
#define _RE_MATERIAL_H_

#include "Texture.glsl"
#include "Constants.glsl"

struct Material
{
	vec4 diffuseColor;

	vec3 emissionColor;
	float metallic;

	float roughness;
	float alphaCutoff;
	float clearCoat;
	float clearCoatRoughness;

	float transmission;
	float specularWeight;
	uint diffuseMap;
	uint normalMap;

	uint metallicRoughnessMap;
	uint occlusionMap;
	uint transmissionMap;
	uint emissionMap;

	uint clearCoatNormalMap;
	uint clearCoatRoughnessMap;
	uint clearCoatMap;
	uint alphaMaskMap;
};

layout(std430, buffer_reference) readonly buffer MaterialBuffer
{
	Material data;
};

#endif /* _RE_MATERIAL_H_ */
