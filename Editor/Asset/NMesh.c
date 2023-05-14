#include <stdio.h>
#include <assert.h>

#include <meshoptimizer.h>

#include <System/Log.h>
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Editor/Asset/Asset.h>
#include <Asset/NMesh.h>

#define ED_NMESH_MOD	"EdNMesh"

void
EdAsset_OptimizeNMesh(struct NMesh *nm)
{
	/*uint32_t *idxBuffer = Sys_Alloc(sizeof(*idxBuffer), nm->indexCount, MH_Editor);

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
	Sys_Free(idxBuffer);*/
}

bool
EdAsset_LoadNMesh(struct NMesh *nm, const char *path)
{
	ASSET_READ_INIT();

	struct NeStream s, *stm = &s;
	E_FileStream(path, IO_READ, stm);

	ASSET_CHECK_GUARD(NMESH_4_HEADER);

	while (!E_EndOfStream(stm)) {
		ASSET_READ_ID();

		if (a.id == NMESH_VTX_ID) {
			nm->vertices = Sys_Alloc(a.size, 1, MH_Editor);
			E_ReadStream(stm, nm->vertices, a.size);
			nm->vertexCount = a.size / sizeof(*nm->vertices);
		} else if (a.id == NMESH_VTXC_ID) {
			if (a.size != sizeof(nm->vertexCount))
				goto error;

			E_ReadStream(stm, &nm->vertexCount, sizeof(nm->vertexCount));
		} else if (a.id == NMESH_IDX_ID) {
			E_ReadStream(stm, &nm->indexType, sizeof(nm->indexType));
			nm->indices = (uint8_t *)Sys_Alloc(a.size, 1, MH_Editor);
			E_ReadStream(stm, nm->indices, a.size);

			switch (nm->indexType) {
			case IT_UINT_16: nm->indexSize = sizeof(uint16_t); break;
			case IT_UINT_32: nm->indexSize = sizeof(uint32_t); break;
			}

			nm->indexCount = a.size / (uint32_t)nm->indexSize;
		} else if (a.id == NMESH_JOINT_ID) {
			nm->joints = Sys_Alloc(sizeof(*nm->joints), a.size, MH_Editor);
			E_ReadStream(stm, nm->joints, sizeof(*nm->joints) * a.size);
			nm->jointCount = a.size;
		} else if (a.id == NMESH_WEIGHT_ID) {
			nm->vertexWeights = Sys_Alloc(sizeof(*nm->vertexWeights), a.size, MH_Editor);
			E_ReadStream(stm, nm->vertexWeights, sizeof(*nm->vertexWeights) * a.size);
			nm->vertexWeightCount = a.size;
		} else if (a.id == NMESH_NODE_ID) {
			nm->nodes = Sys_Alloc(sizeof(*nm->nodes), a.size, MH_Editor);
			E_ReadStream(stm, nm->nodes, sizeof(*nm->nodes) * a.size);
			nm->nodeCount = a.size;
		} else if (a.id == NMESH_INVMAT_ID) {
			nm->inverseBindMatrices = Sys_Alloc(sizeof(*nm->inverseBindMatrices), a.size, MH_Editor);
			E_ReadStream(stm, nm->inverseBindMatrices, sizeof(*nm->inverseBindMatrices) * a.size);
			nm->invMatCount = a.size;
		} else if (a.id == NMESH_INVT_ID) {
			if (a.size != 1)
				goto error;
			E_ReadStream(stm, &nm->inverseTransform, sizeof(nm->inverseTransform));
		} else if (a.id == NMESH_MESH_ID) {
			nm->meshCount = a.size;
			nm->meshes = Sys_Alloc(sizeof(*nm->meshes), nm->meshCount, MH_Editor);
			E_ReadStream(stm, nm->meshes, sizeof(*nm->meshes) * nm->meshCount);
		} else if (a.id == NMESH_MORPH_INFO_ID) {
			nm->morphs = Sys_Alloc(a.size, 1, MH_Editor);
			E_ReadStream(stm, nm->morphs, a.size);
			nm->morphCount = a.size / sizeof(*nm->morphs);
		} else if (a.id == NMESH_MORPH_DELTA_ID) {
			nm->morphDeltas = Sys_Alloc(a.size, 1, MH_Editor);
			E_ReadStream(stm, nm->morphDeltas, a.size);
			nm->morphDeltaCount = a.size / sizeof(*nm->morphDeltas);
		} else if (a.id == NMESH_END_ID) {
			E_SeekStream(stm, -((int64_t)sizeof(a)), IO_SEEK_CUR);
			break;
		} else {
			Sys_LogEntry(ED_NMESH_MOD, LOG_WARNING, "Unknown section id = 0x%x, size = %d", a.id, a.size);
			E_SeekStream(stm, a.size, IO_SEEK_CUR);
		}

		ASSET_CHECK_GUARD(NMESH_SEC_FOOTER);
	}

	ASSET_CHECK_GUARD(NMESH_FOOTER);

	E_CloseStream(stm);

	return true;

error:
	E_CloseStream(stm);

/*	Rt_TermArray(&m->skeleton.bones);
	Rt_TermArray(&m->skeleton.nodes);

	Sys_Free(m->cpu.vertices);
	Sys_Free(m->cpu.indices);
	Sys_Free(m->meshes);
	Sys_Free(matName);*/

	return false;
}

bool
EdAsset_SaveNMesh(const struct NMesh *nm, const char *path)
{
	bool rc = false;
	ASSET_WRITE_INIT();

	FILE *fp = fopen(path, "wb");
	if (!fp)
		return false;

	ASSET_WRITE_GUARD(NMESH_4_HEADER);

	ASSET_WRITE_SEC(NMESH_VTX_ID, sizeof(*nm->vertices) * nm->vertexCount);
	if (fwrite(nm->vertices, sizeof(*nm->vertices), nm->vertexCount, fp) != nm->vertexCount)
		goto exit;
	ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

	ASSET_WRITE_SEC(NMESH_VTXC_ID, sizeof(nm->vertexCount));
	if (fwrite(&nm->vertexCount, sizeof(nm->vertexCount), 1, fp) != 1)
		goto exit;
	ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

	ASSET_WRITE_SEC(NMESH_IDX_ID, (uint32_t)nm->indexSize * nm->indexCount);
	uint32_t type = (uint32_t)nm->indexType;
	if (fwrite(&type, sizeof(type), 1, fp) != 1)
		goto exit;

	if (fwrite(nm->indices, nm->indexSize, nm->indexCount, fp) != nm->indexCount)
		goto exit;
	ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

	if (nm->nodeCount) {
		ASSET_WRITE_SEC(NMESH_NODE_ID, nm->nodeCount);
		if (fwrite(nm->nodes, sizeof(*nm->nodes), nm->nodeCount, fp) != nm->nodeCount)
			goto exit;
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);
	}

	if (nm->jointCount) {
		ASSET_WRITE_SEC(NMESH_JOINT_ID, nm->jointCount);
		if (fwrite(nm->joints, sizeof(*nm->joints), nm->jointCount, fp) != nm->jointCount)
			goto exit;
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);
	}

	if (nm->invMatCount) {
		ASSET_WRITE_SEC(NMESH_INVMAT_ID, nm->invMatCount);
		if (fwrite(nm->inverseBindMatrices, sizeof(*nm->inverseBindMatrices), nm->invMatCount, fp) != nm->invMatCount)
			goto exit;
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

		ASSET_WRITE_SEC(NMESH_INVT_ID, 1);
		if (fwrite(&nm->inverseTransform, sizeof(nm->inverseTransform), 1, fp) != 1)
			goto exit;
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);
	}

	if (nm->vertexWeightCount) {
		ASSET_WRITE_SEC(NMESH_WEIGHT_ID, nm->vertexWeightCount);
		if (fwrite(nm->vertexWeights, sizeof(*nm->vertexWeights), nm->vertexWeightCount, fp) != nm->vertexWeightCount)
			goto exit;
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);
	}

	if (nm->morphCount && nm->morphDeltaCount) {
		ASSET_WRITE_SEC(NMESH_MORPH_INFO_ID, sizeof(*nm->morphs) * nm->morphCount);
		if (fwrite(nm->morphs, sizeof(*nm->morphs), nm->morphCount, fp) != nm->morphCount)
			goto exit;
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

		ASSET_WRITE_SEC(NMESH_MORPH_DELTA_ID, sizeof(*nm->morphDeltas) * nm->morphDeltaCount);
		if (fwrite(nm->morphDeltas, sizeof(*nm->morphDeltas), nm->morphDeltaCount, fp) != nm->morphDeltaCount)
			goto exit;
		ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);
	}

	ASSET_WRITE_SEC(NMESH_MESH_ID, nm->meshCount);
	if (fwrite(nm->meshes, sizeof(*nm->meshes), nm->meshCount, fp) != nm->meshCount)
		goto exit;
	ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

	ASSET_WRITE_GUARD(NMESH_FOOTER);

	rc = true;
exit:
	fclose(fp);
	return rc;
}

void
EdAsset_FreeNMesh(struct NMesh *nm)
{
	Sys_Free(nm->vertices);
	Sys_Free(nm->indices);
	Sys_Free(nm->meshes);
	Sys_Free(nm->vertexWeights);
	Sys_Free(nm->nodes);
	Sys_Free(nm->joints);
	Sys_Free(nm->inverseBindMatrices);
	Sys_Free(nm->morphs);
	Sys_Free(nm->morphDeltas);
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
