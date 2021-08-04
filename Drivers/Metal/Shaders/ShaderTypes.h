#ifndef ShaderTypes_h
#define ShaderTypes_h

#include <metal_stdlib>
using namespace metal;

#define NE_BUFFER(x) \
	uint32_t x ## Buffer; \
	uint32_t x ## Offset

struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
};

struct UIVertex
{
	float x, y, u, v;
	float4 color;
};

struct VertexBones
{
	int4 indices;
	float4 weights;
	int boneCount;
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
	float4 cameraPosition;

	float4 sunPosition;
	uint4 enviornmentMap;

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

#endif /* ShaderTypes_h */
