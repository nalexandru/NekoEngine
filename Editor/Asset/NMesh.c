#include <stdio.h>
#include <assert.h>

#include <meshoptimizer/meshoptimizer.h>

#include <Engine/IO.h>
#include <Editor/Editor.h>
#include <Editor/Asset/NMesh.h>

void
Asset_OptimizeNMesh(struct NMesh *nm)
{
	uint32_t *idxBuffer = Sys_Alloc(sizeof(*idxBuffer), nm->indexCount, MH_Editor);

	if (nm->indexType == IT_UINT_32) {
		memcpy(idxBuffer, nm->indices, sizeof(*idxBuffer) * nm->indexCount);
	} else if (nm->indexType == IT_UINT_16) {
		uint16_t *u16idx = (uint16_t *)nm->indices;
		for (uint32_t i = 0; i < nm->indexCount; ++i)
			idxBuffer[i] = u16idx[i];
	}

	uint32_t *newIndices = Sys_Alloc(sizeof(*newIndices), nm->indexCount, MH_Editor);

	for (uint32_t i = 0; i < nm->meshCount; ++i) {
		const struct NMeshSubmesh *subMesh = &nm->meshes[i];

		uint32_t *smIdx = &idxBuffer[subMesh->indexOffset];
		uint32_t *newIdx = &newIndices[subMesh->indexOffset];
		struct NeVertex *smVtx = &nm->vertices[subMesh->vertexOffset];

		meshopt_generateVertexRemap(newIdx, smIdx, subMesh->indexCount, smVtx, subMesh->vertexCount, sizeof(*smVtx));
		meshopt_optimizeVertexCache(newIdx, newIdx, subMesh->indexCount, subMesh->vertexCount);
		meshopt_optimizeOverdraw(newIdx, newIdx, subMesh->indexCount, &smVtx[0].x, subMesh->vertexCount, sizeof(*smVtx), 1.05f);

		if (!nm->boneCount) {
			meshopt_optimizeVertexFetch(smVtx, newIdx, subMesh->indexCount, smVtx, subMesh->vertexCount, sizeof(*smVtx));
		} else {
			uint32_t *remap = Sys_Alloc(sizeof(*remap), subMesh->vertexCount, MH_Editor);
			
			meshopt_optimizeVertexFetchRemap(remap, newIndices, subMesh->indexCount, subMesh->vertexCount);

			meshopt_remapVertexBuffer(smVtx, smVtx, subMesh->vertexCount, sizeof(*smVtx), remap);
			// remap bone vertex data
			meshopt_remapIndexBuffer(newIdx, newIdx, subMesh->indexCount, remap);

			Sys_Free(remap);
		}
	}

	if (nm->indexType == IT_UINT_32) {
		memcpy(nm->indices, newIndices, sizeof(*idxBuffer) * nm->indexCount);
	} else if (nm->indexType == IT_UINT_16) {
		uint16_t *u16idx = (uint16_t *)nm->indices;
		for (uint32_t i = 0; i < nm->indexCount; ++i)
			u16idx[i] = (uint16_t)newIndices[i];
	}

	Sys_Free(newIndices);
	Sys_Free(idxBuffer);
}

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

	if (nm->boneCount && nm->nodeCount) {
		WRITE_SEC(NMESH_BONE_ID, sizeof(*nm->bones) * nm->boneCount);
		fwrite(nm->bones, sizeof(*nm->bones), nm->boneCount, fp);
		WRITE_GUARD(NMESH_SEC_FOOTER);

		WRITE_SEC(NMESH_NODE_ID, sizeof(*nm->nodes) * nm->nodeCount);
		fwrite(nm->nodes, sizeof(*nm->nodes), nm->nodeCount, fp);
		WRITE_GUARD(NMESH_SEC_FOOTER);

		WRITE_SEC(NMESH_INVT_ID, sizeof(nm->globalInverseTransform));
		fwrite(nm->globalInverseTransform, sizeof(nm->globalInverseTransform), 1, fp);
		WRITE_GUARD(NMESH_SEC_FOOTER);
	}

	WRITE_SEC(NMESH_MESH_ID, nm->meshCount);
	fwrite(nm->meshes, sizeof(*nm->meshes), nm->meshCount, fp);
	WRITE_GUARD(NMESH_SEC_FOOTER);

	WRITE_GUARD(NMESH_FOOTER);

	fclose(fp);
}
