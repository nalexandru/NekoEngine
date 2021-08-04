#ifndef _NE_RENDER_MODEL_H_
#define _NE_RENDER_MODEL_H_

#include <Render/Types.h>
#include <Render/Material.h>
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
	Handle materialResource;
};

struct Model
{
	struct Mesh *meshes;
	uint32_t meshCount;
	uint32_t indexType;

	struct {
		BufferHandle vertexBuffer;
		BufferHandle indexBuffer;
	} gpu;

	struct {
		void *vertices;
		uint32_t vertexSize;

		void *indices;
		uint32_t indexSize;

		void *bones;
		uint32_t boneSize;
	} cpu;
};

struct ModelCreateInfo
{
	void *vertices;
	uint32_t vertexSize;

	void *indices;
	uint32_t indexSize;
	uint8_t indexType;

	struct Mesh *meshes;
	const char **materials;
	uint32_t meshCount;

	bool keepData, loadMaterials;
};

#pragma pack(push, 1)
struct ModelInstance
{
	struct mat4 mvp;
	struct mat4 model;
	struct mat4 normal;
	uint64_t vertexAddress;
	uint64_t materialAddress;
};
#pragma pack(pop)

bool Re_CreateModelResource(const char *name, const struct ModelCreateInfo *ci, struct Model *mdl, Handle h);
bool Re_LoadModelResource(struct ResourceLoadInfo *li, const char *args, struct Model *mdl, Handle h);
void Re_UnloadModelResource(struct Model *mdl, Handle h);

#endif /* _NE_RENDER_MODEL_H_ */
