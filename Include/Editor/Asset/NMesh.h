#ifndef _NE_EDITOR_ASSET_NMESH_H_
#define _NE_EDITOR_ASSET_NMESH_H_

#include <Render/Model.h>
#include <Runtime/Array.h>

#define NMESH_4_HEADER		0x0000344853454D4Ellu	// NMESH4
#define NMESH_FOOTER		0x004853454D444E45llu	// ENDMESH
#define NMESH_SEC_FOOTER	0x0054434553444E45llu	// ENDSECT
#define NMESH_VTX_ID		0x00585456u				// VTX
#define NMESH_IDX_ID		0x00584449u				// IDX
#define NMESH_MAT_ID		0x0054414Du				// MAT
#define NMESH_BONE_ID		0x454E4F42u				// BONE
#define NMESH_MESH_ID		0x4853454Du				// MESH

struct NMeshSubmesh
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	char material[256];
};

struct NMeshBone
{
	void *a;
};

struct NMesh
{
	uint32_t vertexCount;
	struct Vertex *vertices;

	enum IndexType indexType;
	size_t indexSize;
	uint32_t indexCount;
	uint8_t *indices;

	uint32_t meshCount;
	struct NMeshSubmesh *meshes;

	uint32_t boneCount;
	struct NMeshBone *bones;
};

void Asset_SaveNMesh(const struct NMesh *nm, const char *path);

#endif /* _NE_EDITOR_ASSET_NMESH_H_ */
