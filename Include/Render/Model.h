#ifndef _RE_MODEL_H_
#define _RE_MODEL_H_

#include <Engine/Types.h>
#include <Runtime/Runtime.h>

#pragma pack(push,1)
struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
};
#pragma pack(pop)

struct Mesh
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	struct Material *mat;
};

struct Model
{
	struct Mesh *meshes;
	uint32_t meshCount;
	
	struct Vertex *vertices;
	uint32_t vertexCount;
	
	uint32_t *indices;
	uint32_t indexCount;
};

#endif /* _RE_MODEL_H_ */
