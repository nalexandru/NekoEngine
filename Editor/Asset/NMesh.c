#include <stdio.h>
#include <assert.h>

#include <Engine/IO.h>
#include <Editor/Editor.h>
#include <Editor/Asset/NMesh.h>

void
Asset_SaveNMesh(const struct NMesh *nm, const char *path)
{
	union {
		uint64_t guard;
		struct {
			uint32_t id;
			uint32_t size;
		};
	} a;

#define WRITE_SEC(i, v) a.id = i; a.size = v; fwrite(&a, sizeof(a), 1, fp)
#define WRITE_GUARD(x) a.guard = x; fwrite(&a, sizeof(a), 1, fp)

	FILE *fp = fopen(path, "wb");
	assert(fp);

	WRITE_GUARD(NMESH_4_HEADER);

	WRITE_SEC(NMESH_VTX_ID, sizeof(*nm->vertices) * nm->vertexCount);
	fwrite(nm->vertices, sizeof(*nm->vertices), nm->vertexCount, fp);
	WRITE_GUARD(NMESH_SEC_FOOTER);

	WRITE_SEC(NMESH_IDX_ID, (uint32_t)nm->indexSize * nm->indexCount);
	uint32_t type = (uint32_t)nm->indexType;
	fwrite(&type, sizeof(type), 1, fp);
	fwrite(nm->indices, nm->indexSize, nm->indexCount, fp);
	WRITE_GUARD(NMESH_SEC_FOOTER);

	if (nm->boneCount) {
		WRITE_SEC(NMESH_BONE_ID, sizeof(*nm->bones) * nm->boneCount);
		fwrite(nm->bones, sizeof(*nm->bones), nm->boneCount, fp);
		WRITE_GUARD(NMESH_SEC_FOOTER);
	}

	WRITE_SEC(NMESH_MESH_ID, nm->meshCount);
	fwrite(nm->meshes, sizeof(*nm->meshes), nm->meshCount, fp);
	WRITE_GUARD(NMESH_SEC_FOOTER);

	WRITE_GUARD(NMESH_FOOTER);

	fclose(fp);
}
