#ifndef Material_h
#define Material_h

#include <metal_stdlib>
using namespace metal;

struct Material
{
	float4 diffuseColor;

	float4 emissionColor;
	//float metallic;

	float roughness;
	float alphaCutoff;
	float clearCoat;
	float clearCoatRoughness;

	float transmission;
	float specularWeight;
	uint32_t diffuseMap;
	uint32_t normalMap;

	uint32_t metallicRoughnessMap;
	uint32_t occlusionMap;
	uint32_t transmissionMap;
	uint32_t emissionMap;

	uint32_t clearCoatNormalMap;
	uint32_t clearCoatRoughnessMap;
	uint32_t clearCoatMap;
	uint32_t alphaMaskMap;
};

#endif /* Material_h */
