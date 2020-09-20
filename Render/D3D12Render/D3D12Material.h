#pragma once

#include <Engine/Types.h>

struct D3D12Material
{
	float diffuseColor[4];
	float emissiveColor[4];
	float roughness;
	float metallic;
	uint32_t textures[10];
};
