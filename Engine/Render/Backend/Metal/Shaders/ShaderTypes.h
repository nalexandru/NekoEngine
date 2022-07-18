#ifndef ShaderTypes_h
#define ShaderTypes_h

#include <metal_stdlib>
using namespace metal;

#define NE_BUFFER(x) \
	uint32_t x ## Buffer; \
	uint32_t x ## Offset

struct Vertex
{
	float3 position [[attribute(0)]];
	float3 normal [[attribute(1)]];
	float3 tangent [[attribute(2)]];
	float2 uv [[attribute(3)]];
	float4 color [[attribute(4)]];
};

struct UIVertex
{
//	float4 posAndUV [[attribute[0]]]
	float x, y, u, v;
	float4 color;
};

struct VertexBones
{
	int4 indices [[attribute(5)]];
	float4 weights [[attribute(6)]];
	int boneCount [[attribute(7)]];
};

struct Light
{
	float x, y, z;
	uint type;

	float4 direction;
	float4 color;

	float innerRadius, outerRadius;
	float innerCutoff, outerCutoff;
};

struct Scene
{
	float4x4 viewProjection;
	float4x4 inverseProjection;
	float4 cameraPosition;

	float4 sunPosition;
	uint4 enviornmentMaps;

	uint lightCount;
	uint xTileCount;

	NE_BUFFER(instances);

	float exposure;
	float gamma;
	float invGamma;
	uint sampleCount;

	Light lightStart;
};

struct ModelInstance
{
	float4x4 mvp;
	float4x4 model;
	float4x4 normal;
	NE_BUFFER(vertex);
	NE_BUFFER(material);
};

#define LT_DIRECTIONAL	0
#define LT_POINT		1
#define LT_SPOT			2

struct ShaderArguments
{
	const array<sampler, 3> samplers [[ id(0) ]];

#ifndef SA_CUBE_TEXTURES
	const array<texture2d<float>, 65535> textures [[ id(3) ]];
#else
	const array<texturecube<float>, 65535> cubeTextures [[ id(3) ]];
#endif

#ifdef SA_WRITABLE_BUFFERS
	const array<device uint8_t *, 65535> buffers [[ id(65538) ]];
#else
	const array<constant uint8_t *, 65535> buffers [[ id(65538) ]];
#endif
};

#endif /* ShaderTypes_h */
