#include <stdio.h>
#include <assert.h>

#include <meshoptimizer/meshoptimizer.h>

#include <Engine/IO.h>
#include <Editor/Asset/Asset.h>
#include <Asset/NMesh.h>

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

		if (!nm->jointCount) {
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
	ASSET_WRITE_INIT();

	FILE *fp = fopen(path, "wb");
	assert(fp);

	ASSET_WRITE_GUARD(NMESH_4_HEADER);

	ASSET_WRITE_SEC(NMESH_VTX_ID, sizeof(*nm->vertices) * nm->vertexCount);
	fwrite(nm->vertices, sizeof(*nm->vertices), nm->vertexCount, fp);
	ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

	ASSET_WRITE_SEC(NMESH_VTXC_ID, sizeof(nm->vertexCount));
	fwrite(&nm->vertexCount, sizeof(nm->vertexCount), 1, fp);
	ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

	ASSET_WRITE_SEC(NMESH_IDX_ID, (uint32_t)nm->indexSize * nm->indexCount);
	uint32_t type = (uint32_t)nm->indexType;
	fwrite(&type, sizeof(type), 1, fp);
	fwrite(nm->indices, nm->indexSize, nm->indexCount, fp);
	ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

	if (nm->jointCount && nm->nodeCount) {
		ASSET_WRITE_SEC(NMESH_BONE_ID, sizeof(*nm->joints) * nm->jointCount);
		fwrite(nm->joints, sizeof(*nm->joints), nm->jointCount, fp);
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

		ASSET_WRITE_SEC(NMESH_NODE_ID, sizeof(*nm->nodes) * nm->nodeCount);
		fwrite(nm->nodes, sizeof(*nm->nodes), nm->nodeCount, fp);
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

		ASSET_WRITE_SEC(NMESH_INVT_ID, sizeof(nm->globalInverseTransform));
		fwrite(nm->globalInverseTransform, sizeof(nm->globalInverseTransform), 1, fp);
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);
	}

	if (nm->vertexWeightCount) {
		ASSET_WRITE_SEC(NMESH_WEIGHT_ID, sizeof(*nm->vertexWeights) * nm->vertexWeightCount);
		fwrite(nm->vertexWeights, sizeof(*nm->vertexWeights), nm->vertexWeightCount, fp);
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);
	}

	if (nm->morphCount && nm->morphDeltaCount) {
		ASSET_WRITE_SEC(NMESH_MORPH_INFO_ID, sizeof(*nm->morphs) * nm->morphCount);
		fwrite(nm->morphs, sizeof(*nm->morphs), nm->morphCount, fp);
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

		ASSET_WRITE_SEC(NMESH_MORPH_DELTA_ID, sizeof(*nm->morphDeltas) * nm->morphDeltaCount);
		fwrite(nm->morphDeltas, sizeof(*nm->morphDeltas), nm->morphDeltaCount, fp);
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);
	}

	ASSET_WRITE_SEC(NMESH_MESH_ID, nm->meshCount);
	fwrite(nm->meshes, sizeof(*nm->meshes), nm->meshCount, fp);
	ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

	ASSET_WRITE_GUARD(NMESH_FOOTER);

	fclose(fp);
}

/* NekoEditor
 *
 * NMesh.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
