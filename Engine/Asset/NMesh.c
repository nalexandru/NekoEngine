#include <assert.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Render/Model.h>
#include <Runtime/Array.h>
#include <System/Log.h>
#include <Animation/Skeleton.h>

#define NMESH_MOD	"NMesh"

// all values are little-endian
#define NMESH_4_HEADER		0x0000344853454D4Ellu	// NMESH4
#define NMESH_FOOTER		0x004853454D444E45llu	// ENDMESH
#define NMESH_SEC_FOOTER	0x0054434553444E45llu	// ENDSECT
#define NMESH_VTX_ID		0x00585456u				// VTX
#define NMESH_IDX_ID		0x00584449u				// IDX
#define NMESH_MAT_ID		0x0054414Du				// MAT
#define NMESH_BONE_ID		0x454E4F42u				// BONE
#define NMESH_WEIGHT_ID		0x54484757u				// WGHT
#define NMESH_NODE_ID		0x45444F4Eu				// NODE
#define NMESH_INVT_ID		0x54564E49u				// INVT
#define NMESH_MESH_ID		0x4853454Du				// MESH
#define NMESH_END_ID		0x4D444E45u				// ENDM

#define READ_ID()															\
	if (E_ReadStream(stm, &a.guard, sizeof(a.guard)) != sizeof(a.guard))	\
		goto error 

#define CHECK_GUARD(val)													\
	if (E_ReadStream(stm, &a.guard, sizeof(a.guard)) != sizeof(a.guard))	\
		goto error;															\
	if (a.guard != val)														\
		goto error

struct NMeshInfo
{
	uint32_t primitiveType;
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	char material[256];
};

bool
E_LoadNMeshAsset(struct NeStream *stm, struct NeModel *m)
{
	union {
		uint64_t guard;
		struct {
			uint32_t id;
			uint32_t size;
		};
	} a;

	char *matName = Sys_Alloc(sizeof(*matName), 258, MH_Asset);

	CHECK_GUARD(NMESH_4_HEADER);

	while (!E_EndOfStream(stm)) {
		READ_ID();

		if (a.id == NMESH_VTX_ID) {
			m->cpu.vertexSize = a.size;
			m->cpu.vertices = Sys_Alloc(a.size, 1, MH_Asset);
			E_ReadStream(stm, m->cpu.vertices, a.size);
		} else if (a.id == NMESH_IDX_ID) {
			E_ReadStream(stm, &m->indexType, sizeof(m->indexType));
			m->cpu.indexSize = a.size;
			m->cpu.indices = Sys_Alloc(a.size, 1, MH_Asset);
			E_ReadStream(stm, m->cpu.indices, a.size);
		} else if (a.id == NMESH_BONE_ID) {
			size_t count = a.size / sizeof(struct NeBone);
			Rt_InitArray(&m->skeleton.bones, count, sizeof(struct NeBone), MH_Asset);
			E_ReadStream(stm, m->skeleton.bones.data, a.size);
		} else if (a.id == NMESH_WEIGHT_ID) {
			m->cpu.vertexWeightSize = a.size;
			m->cpu.vertexWeights = Sys_Alloc(a.size, 1, MH_Asset);
			E_ReadStream(stm, m->cpu.vertexWeights, a.size);
		} else if (a.id == NMESH_NODE_ID) {
			assert(!"Skeleton nodes not implemented");
			size_t count = a.size / sizeof(struct NeSkeletonNode);
			Rt_InitArray(&m->skeleton.nodes, count, sizeof(struct NeSkeletonNode), MH_Asset);
			E_ReadStream(stm, m->skeleton.nodes.data, a.size);
		} else if (a.id == NMESH_INVT_ID) {
			if (a.size != sizeof(m->skeleton.globalInverseTransform.m))
				goto error;

			E_ReadStream(stm, m->skeleton.globalInverseTransform.m, sizeof(m->skeleton.globalInverseTransform.m));
		} else if (a.id == NMESH_MESH_ID) {
			struct NMeshInfo info = { 0 };

			m->meshCount = a.size;
			m->meshes = Sys_Alloc(sizeof(*m->meshes), m->meshCount, MH_Asset);

			for (uint32_t i = 0; i < a.size; ++i) {
				if (E_ReadStream(stm, &info, sizeof(info)) != sizeof(info))
					goto error;

				memset(matName, 0x0, 258);
				snprintf(matName, 258, "%s:%u", info.material, info.primitiveType);

				m->meshes[i].type = info.primitiveType;
				m->meshes[i].vertexOffset = info.vertexOffset;
				m->meshes[i].vertexCount = info.vertexCount;
				m->meshes[i].indexOffset = info.indexOffset;
				m->meshes[i].indexCount = info.indexCount;
				m->meshes[i].materialResource = E_LoadResource(matName, RES_MATERIAL);

				Re_BuildMeshBounds(&m->meshes[i].bounds, m->cpu.vertices, info.vertexOffset, info.vertexCount);
			}
		} else if (a.id == NMESH_END_ID) {
			E_StreamSeek(stm, -((int64_t)sizeof(a)), IO_SEEK_CUR);
			break;
		} else {
			Sys_LogEntry(NMESH_MOD, LOG_WARNING, "Unknown section id = 0x%x, size = %d", a.id, a.size);
			E_StreamSeek(stm, a.size, IO_SEEK_CUR);
		}

		CHECK_GUARD(NMESH_SEC_FOOTER);
	}

	CHECK_GUARD(NMESH_FOOTER);

	Sys_Free(matName);

	return true;

error:
	Rt_TermArray(&m->skeleton.bones);
	Rt_TermArray(&m->skeleton.nodes);

	Sys_Free(m->cpu.vertices);
	Sys_Free(m->cpu.indices);
	Sys_Free(m->meshes);
	Sys_Free(matName);

	return false;
}
