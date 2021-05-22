#ifndef _RE_MATERIAL_H_
#define _RE_MATERIAL_H_

struct Material
{
	vec4 diffuseColor;

	vec3 emissionColor;
	float metallic;

	float roughness;
	//float _padding[3];

	uint diffuseMap;
	uint normalMap;
	uint metallicMap;
	uint roughnessMap;
};

layout(std430, buffer_reference) readonly buffer MaterialBuffer
{
	Material data;
};

#endif /* _RE_MATERIAL_H_ */
