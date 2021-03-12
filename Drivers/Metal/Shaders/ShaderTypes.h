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

#define LT_DIRECTIONAL	0
#define LT_POINT		1
#define LT_SPOT			2

#endif /* ShaderTypes_h */
