#ifndef _RE_MODEL_H_
#define _RE_MODEL_H_

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

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
	uint32_t firstVertex;
	uint32_t vertexCount;
	uint32_t firstIndex;
	uint32_t indexCount;
};

struct Model
{
	struct Mesh *meshes;
	struct MaterialInstance *materialInstances;
	uint32_t numMeshes;
	
	struct Vertex *vertices;
	uint32_t numVertices;
	
	uint32_t *indices;
	uint32_t numIndices;

	wchar_t **materialNames;

	uint8_t renderDataStart;
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

bool Re_CreateModel(const char *name, const struct ModelCreateInfo *ci, struct Model *data, Handle h);
bool Re_LoadModel(struct ResourceLoadInfo *li, const char *args, struct Model *model, Handle h);
void Re_UnloadModel(struct Model *model, Handle h);

#ifdef __cplusplus
}
#endif

#endif /* _RE_MODEL_H_ */
