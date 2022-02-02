#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#define NMESH_4_HEADER		0x0000344853454D4Ellu	// NMESH4
#define NMESH_FOOTER		0x004853454D444E45llu	// ENDMESH
#define NMESH_SEC_FOOTER	0x0054434553444E45llu	// ENDSECT
#define NMESH_VTX_ID		0x00585456u				// VTX
#define NMESH_IDX_ID		0x00584449u				// IDX
#define NMESH_MAT_ID		0x0054414Du				// MAT
#define NMESH_BONE_ID		0x454E4F42u				// BONE
#define NMESH_MESH_ID		0x4853454Du				// MESH
#define NMESH_END_ID		0x4D444E45u				// ENDM

#define READ_ID()															\
	if (fread(&a.guard, 1, sizeof(a.guard), fp) != sizeof(a.guard))			\
		goto error 

#define CHECK_GUARD(val)													\
	if (fread(&a.guard, 1, sizeof(a.guard), fp) != sizeof(a.guard))			\
		goto error;															\
	if (a.guard != val)														\
		goto error

struct NMeshInfo
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	char material[256];
};

struct NMesh4Vertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
};

struct NMesh4aVertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
	float r, g, b, a;
};

struct NMesh
{
	uint32_t vertexSize;
	uint32_t vertexCount;
	struct NMesh4Vertex *vertices;
	struct NMesh4aVertex *vertices4a;

	uint32_t indexSize;
	uint32_t indexType;
	void *indices;

	uint32_t boneSize;
	void *bones;

	uint32_t meshCount;
	struct NMeshInfo *meshes;
};

int
main(int argc, char *argv[])
{
	struct NMesh m = { 0 };
	union {
		uint64_t guard;
		struct {
			uint32_t id;
			uint32_t size;
		};
	} a;

	FILE *fp = fopen(argv[1], "rb");
	if (!fp)
		return -1;

	CHECK_GUARD(NMESH_4_HEADER);

	while (!feof(fp)) {
		READ_ID();

		if (a.id == NMESH_VTX_ID) {
			m.vertexSize = a.size;
			m.vertices = calloc(1, a.size);
			fread(m.vertices, a.size, 1, fp);
	//		E_ReadStream(stm, m->cpu.vertices, a.size);
		} else if (a.id == NMESH_IDX_ID) {
			fread(&m.indexType, sizeof(m.indexType), 1, fp);
	//		E_ReadStream(stm, &m->indexType, sizeof(m->indexType));
			m.indexSize = a.size;
			m.indices = calloc(1, a.size);
			fread(m.indices, a.size, 1, fp);
	//		E_ReadStream(stm, m.indices, a.size);
		} else if (a.id == NMESH_BONE_ID) {
			m.boneSize = a.size;
			m.bones = calloc(1, a.size);
			fread(m.bones, a.size, 1, fp);
	//		E_ReadStream(stm, m.bones, a.size);
		} else if (a.id == NMESH_MESH_ID) {
			m.meshCount = a.size;
			m.meshes = calloc(m.meshCount, sizeof(*m.meshes));

			for (uint32_t i = 0; i < a.size; ++i)
				fread(&m.meshes[i], sizeof(struct NMeshInfo), 1, fp);
		} else if (a.id == NMESH_END_ID) {
			fseek(fp, -((int64_t)sizeof(a)), SEEK_CUR);
	//		E_StreamSeek(stm, -((int64_t)sizeof(a)), IO_SEEK_CUR);
			break;
		} else {
	//		Sys_LogEntry(NMESH_MOD, LOG_WARNING, L"Unknown section id = 0x%x, size = %d", a.id, a.size);
	//		E_StreamSeek(stm, a.size, IO_SEEK_CUR);
			fseek(fp, a.size, SEEK_CUR);
		}

		CHECK_GUARD(NMESH_SEC_FOOTER);
	}

	CHECK_GUARD(NMESH_FOOTER);

	fclose(fp);

	m.vertexCount = m.vertexSize / sizeof(struct NMesh4Vertex);
	m.vertices4a = calloc(m.vertexCount, sizeof(struct NMesh4aVertex));

	for (size_t i = 0; i < m.vertexCount; ++i) {
		m.vertices4a[i].x = m.vertices[i].x;
		m.vertices4a[i].y = m.vertices[i].y;
		m.vertices4a[i].z = m.vertices[i].z;

		m.vertices4a[i].nx = m.vertices[i].nx;
		m.vertices4a[i].ny = m.vertices[i].ny;
		m.vertices4a[i].nz = m.vertices[i].nz;

		m.vertices4a[i].tx = m.vertices[i].tx;
		m.vertices4a[i].ty = m.vertices[i].ty;
		m.vertices4a[i].tz = m.vertices[i].tz;

		m.vertices4a[i].u = m.vertices[i].u;
		m.vertices4a[i].v = m.vertices[i].v;

		m.vertices4a[i].r = 1.f;
		m.vertices4a[i].g = 1.f;
		m.vertices4a[i].b = 1.f;
		m.vertices4a[i].a = 1.f;
	}

/////

#define WRITE_SEC(i, v) a.id = i; a.size = v; fwrite(&a, sizeof(a), 1, fp)
#define WRITE_GUARD(x) a.guard = x; fwrite(&a, sizeof(a), 1, fp)

	fp = fopen("out.nmesh4a", "wb");
	assert(fp);

	WRITE_GUARD(NMESH_4_HEADER);

	WRITE_SEC(NMESH_VTX_ID, sizeof(*m.vertices4a) * m.vertexCount);
	fwrite(m.vertices4a, sizeof(*m.vertices4a), m.vertexCount, fp);
	WRITE_GUARD(NMESH_SEC_FOOTER);

	WRITE_SEC(NMESH_IDX_ID, (uint32_t)m.indexSize);
	uint32_t type = (uint32_t)m.indexType;
	fwrite(&type, sizeof(type), 1, fp);
	fwrite(m.indices, m.indexSize, 1, fp);
	WRITE_GUARD(NMESH_SEC_FOOTER);

	if (m.boneSize) {
		WRITE_SEC(NMESH_BONE_ID, m.boneSize);
		fwrite(m.bones, m.boneSize, 1, fp);
		WRITE_GUARD(NMESH_SEC_FOOTER);
	}

	WRITE_SEC(NMESH_MESH_ID, m.meshCount);
	fwrite(m.meshes, sizeof(*m.meshes), m.meshCount, fp);
	WRITE_GUARD(NMESH_SEC_FOOTER);

	WRITE_GUARD(NMESH_FOOTER);

	fclose(fp);

	return 0;

error:
	printf("error\n");

	free(m.vertices);
	free(m.vertices4a);
	free(m.indices);
	free(m.bones);
	free(m.meshes);

	return -1;
}
