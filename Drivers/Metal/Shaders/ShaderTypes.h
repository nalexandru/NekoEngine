#ifndef ShaderTypes_h
#define ShaderTypes_h

struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
};

struct VertexBones
{
	int4 indices;
	float4 weights;
	int boneCount;
};

struct Light
{
	float4 position;	// xyz - position, w - type
	float4 direction;	// xyz - direction
	float4 color;		// xyz - color, w - intensity
	float4 data;		// x - inner radius, y - outer radius, z - inner spot cutoff, w - outer spot cutoff
};

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
	//uint32_t __padding;
};

#define LT_DIRECTIONAL	0
#define LT_POINT		1
#define LT_SPOT			2

#endif /* ShaderTypes_h */
