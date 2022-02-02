#ifndef _NE_RENDER_MODEL_H_
#define _NE_RENDER_MODEL_H_

#include <Render/Types.h>
#include <Render/Material.h>
#include <Runtime/Runtime.h>

#pragma pack(push,1)
struct NeVertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
	float r, g, b, a;
};

struct NeVertexWeight
{
	int32_t i1, i2, i3, i4, i5, i6, i7;
	float w1, w2, w3, w4, w5, w6, w7;
	uint32_t boneCount;
	uint32_t reserved;
};
#pragma pack(pop)

struct NeMesh
{
	enum NePrimitiveType type;
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	NeHandle materialResource;
};

struct NeModel
{
	struct NeMesh *meshes;
	uint32_t meshCount;
	enum NeIndexType indexType;
	bool dynamic;

	struct {
		NeBufferHandle vertexBuffer, vertexWeightBuffer;
		NeBufferHandle indexBuffer;
	} gpu;

	struct {
		void *vertices;
		uint32_t vertexSize;

		void *indices;
		uint32_t indexSize;

		void *vertexWeights;
		uint32_t vertexWeightSize;
	} cpu;

	struct {
		struct mat4 globalInverseTransform;
		struct NeArray bones;
		struct NeArray nodes;
	} skeleton;
};

struct NeModelCreateInfo
{
	void *vertices;
	uint32_t vertexSize;

	void *indices;
	uint32_t indexSize;
	uint8_t indexType;

	struct NeBone *bones;
	uint32_t boneSize;

	struct NeVertexWeight *vertexWeights;
	uint32_t vertexWeightSize;

	struct NeMesh *meshes;
	const char **materials;
	uint32_t meshCount;

	bool keepData, loadMaterials, dynamic;
};

#pragma pack(push, 1)
struct NeModelInstance
{
	struct mat4 mvp;
	struct mat4 model;
	struct mat4 normal;
	uint64_t vertexAddress;
	uint64_t materialAddress;
} NE_ALIGN(16);
#pragma pack(pop)

bool Re_CreateModelResource(const char *name, const struct NeModelCreateInfo *ci, struct NeModel *mdl, NeHandle h);
bool Re_LoadModelResource(struct NeResourceLoadInfo *li, const char *args, struct NeModel *mdl, NeHandle h);
void Re_UnloadModelResource(struct NeModel *mdl, NeHandle h);

#endif /* _NE_RENDER_MODEL_H_ */
