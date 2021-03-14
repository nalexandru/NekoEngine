#ifndef _RE_MODEL_H_
#define _RE_MODEL_H_

#include <Engine/Types.h>
#include <Runtime/Runtime.h>

#define RES_MODEL	"Model"

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

struct ModelCreateInfo
{
	struct Vertex *vertices;
	uint32_t vertexCount;
	
	uint32_t *indices;
	uint32_t indexCount;
	
	struct Mesh *meshes;
	const wchar_t **materials;
	uint32_t meshCount;
};

bool Re_CreateModelResource(const char *name, const struct ModelCreateInfo *ci, struct Model *mdl, Handle h);
bool Re_LoadModelResource(struct ResourceLoadInfo *li, const char *args, struct Model *mdl, Handle h);
void Re_UnloadModelResource(struct Model *mdl, Handle h);

#endif /* _RE_MODEL_H_ */
