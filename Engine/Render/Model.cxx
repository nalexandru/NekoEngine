#include <Math/Math.h>
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Render/Model.h>
#include <Render/Render.h>
#include <System/Memory.h>
#include <Animation/Skeleton.h>

static inline bool InitModel(struct NeModel *mdl);

bool
Re_CreateModelResource(const char *name, const struct NeModelCreateInfo *ci, struct NeModel *mdl, NeHandle h)
{
	const size_t meshSize = sizeof(*mdl->meshes) * ci->meshCount;

	mdl->dynamic = ci->dynamic;

	if (!(mdl->cpu.vertices = Sys_Alloc(ci->vertexSize, 1, MH_Render)))
		goto error;
	memcpy(mdl->cpu.vertices, ci->vertices, ci->vertexSize);
	mdl->cpu.vertexSize = ci->vertexSize;

	if (!(mdl->cpu.indices = Sys_Alloc(ci->indexSize, 1, MH_Render)))
		goto error;
	memcpy(mdl->cpu.indices, ci->indices, ci->indexSize);
	mdl->cpu.indexSize = ci->indexSize;
	mdl->indexType = ci->indexType;

	if (ci->vertexWeights) {
		if (!(mdl->cpu.vertexWeights = (struct NeVertexWeight *)Sys_Alloc(sizeof(*mdl->cpu.vertexWeights), ci->vertexWeightCount, MH_Render)))
			goto error;
		memcpy(mdl->cpu.vertexWeights, ci->vertexWeights, sizeof(*mdl->cpu.vertexWeights) * ci->vertexWeightCount);
		mdl->cpu.vertexWeightCount = ci->vertexWeightCount;
	} else {
		mdl->cpu.vertexWeights = NULL;
		mdl->cpu.vertexWeightCount = 0;
	}

	if (!(mdl->meshes = (struct NeMesh *)Sys_Alloc(meshSize, 1, MH_Render)))
		goto error;
	memcpy(mdl->meshes, ci->meshes, meshSize);
	mdl->meshCount = ci->meshCount;

	if (ci->materials) {
		for (uint32_t i = 0; i < mdl->meshCount; ++i)
			mdl->meshes[i].materialResource = E_LoadResource(ci->materials[i], RES_MATERIAL);
	}

	if (!ci->keepData) {
		Sys_Free(ci->vertices);
		Sys_Free(ci->indices);
		Sys_Free(ci->meshes);
	}

	M_Store(&mdl->skeleton.inverseTransform, XMMatrixIdentity());

	return InitModel(mdl);

error:
	Sys_Free(mdl->cpu.vertices);
	Sys_Free(mdl->cpu.indices);
	Sys_Free(mdl->meshes);

	return false;
}

bool
Re_LoadModelResource(struct NeResourceLoadInfo *li, const char *args, struct NeModel *mdl, NeHandle h)
{
	M_Store(&mdl->skeleton.inverseTransform, XMMatrixIdentity());

	if (!Asset_LoadMesh(&li->stm, mdl))
		return false;

	if (args && strstr(args, "dynamic"))
		mdl->dynamic = true;

	Re_BuildMeshBounds(&mdl->bounds, (struct NeVertex *)mdl->cpu.vertices, 0, mdl->cpu.vertexSize / sizeof(struct NeVertex));

	return InitModel(mdl);
}

void
Re_UnloadModelResource(struct NeModel *mdl, NeHandle h)
{
	for (uint32_t i = 0; i < mdl->meshCount; ++i)
		E_UnloadResource(mdl->meshes[i].materialResource);

	Re_Destroy(mdl->gpu.vertexBuffer);
	Re_Destroy(mdl->gpu.indexBuffer);

	struct NeSkeletonNode *n;
	Rt_ArrayForEach(n, &mdl->skeleton.nodes, struct NeSkeletonNode *)
		Rt_TermArray(&n->children);

	Rt_TermArray(&mdl->skeleton.joints);
	Rt_TermArray(&mdl->skeleton.nodes);

	Sys_Free(mdl->skeleton.inverseTransforms);
	Sys_Free(mdl->cpu.vertexWeights);
	Sys_Free(mdl->cpu.vertices);
	Sys_Free(mdl->morph.deltas);
	Sys_Free(mdl->cpu.indices);
	Sys_Free(mdl->morph.info);
	Sys_Free(mdl->meshes);

	if (mdl->gpu.vertexWeightBuffer)
		Re_Destroy(mdl->gpu.vertexWeightBuffer);
}

void
Re_BuildMeshBounds(struct NeBounds *b, const struct NeVertex *vertices, uint32_t startVertex, uint32_t vertexCount)
{
	XMVECTOR min = XMVectorZero();
	XMVECTOR max = XMVectorZero();
	XMVECTOR center = XMVectorZero();

	const uint32_t endVertex = startVertex + vertexCount;
	for (uint32_t i = startVertex; i < endVertex; ++i) {
		const XMVECTOR v = XMLoadFloat3((XMFLOAT3 *)&vertices[i].x);

		min = XMVectorMin(min, v);
		max = XMVectorMax(max, v);
		center = XMVectorAdd(center, v);
	}

	center = XMVectorDivide(center, XMVectorReplicate((float)vertexCount));

	XMStoreFloat3((XMFLOAT3 *)&b->sphere.center, center);
	XMStoreFloat3((XMFLOAT3 *)&b->aabb.min, min);
	XMStoreFloat3((XMFLOAT3 *)&b->aabb.max, max);

	for (uint32_t i = startVertex; i < endVertex; ++i) {
		const XMVECTOR v = XMLoadFloat3((XMFLOAT3 *)&vertices[i].x);
	
		const float dist = XMVectorGetX(XMVector3Length(XMVectorSubtract(center, v)));
		if (dist > b->sphere.radius)
			b->sphere.radius = dist;
	}
}

static inline bool
InitModel(struct NeModel *mdl)
{
	struct NeBufferCreateInfo bci =
	{
		.desc =
		{
			.size = mdl->cpu.vertexSize,
			.usage = BU_VERTEX_BUFFER | BU_STORAGE_BUFFER | BU_TRANSFER_DST,
			.memoryType = MT_GPU_LOCAL
		},
		.data = mdl->cpu.vertices,
		.dataSize = mdl->cpu.vertexSize,
		.keepData = true
	};
	if (!Re_CreateBuffer(&bci, &mdl->gpu.vertexBuffer))
		return false;

	if (mdl->cpu.vertexWeights) {
		bci.data = mdl->cpu.vertexWeights;
		bci.desc.size = bci.dataSize = mdl->cpu.vertexWeightCount * sizeof(*mdl->cpu.vertexWeights);
		if (!Re_CreateBuffer(&bci, &mdl->gpu.vertexWeightBuffer))
			return false;
	}

	bci.desc.usage |= BU_INDEX_BUFFER;
	bci.data = mdl->cpu.indices;
	bci.desc.size = bci.dataSize = mdl->cpu.indexSize;
	if (!Re_CreateBuffer(&bci, &mdl->gpu.indexBuffer))
		return false;

	if (!mdl->dynamic) {
		Sys_Free(mdl->cpu.vertices); mdl->cpu.vertices = NULL;
		Sys_Free(mdl->cpu.indices); mdl->cpu.indices = NULL;
		Sys_Free(mdl->cpu.vertexWeights); mdl->cpu.vertexWeights = NULL;
	}

	return true;
}

bool
Re_CreateMorphPackResource(const char *name, const struct NeMorphPackCreateInfo *ci, struct NeMorphPack *mp, NeHandle h)
{
	mp->count = ci->count;
	mp->morphs = (struct NeMorph *)Sys_Alloc(sizeof(*mp->morphs), mp->count, MH_Asset);
	if (!mp->morphs)
		return false;

	memcpy(mp->morphs, ci->morphs, sizeof(*mp->morphs) * mp->count);

	mp->deltaCount = ci->deltaCount;
	mp->deltas = (struct NeMorphDelta *)Sys_Alloc(sizeof(*mp->deltas), mp->deltaCount, MH_Asset);
	if (!mp->deltas) {
		Sys_Free(mp->morphs);
		return false;
	}

	memcpy(mp->deltas, ci->deltas, sizeof(*mp->deltas) * mp->deltaCount);

	if (ci->keepData)
		return true;

	Sys_Free(ci->morphs);
	Sys_Free(ci->deltas);

	return true;
}

bool
Re_LoadMorphPackResource(struct NeResourceLoadInfo *li, const char *args, struct NeMorphPack *mp, NeHandle h)
{
	return Asset_LoadMorphPack(&li->stm, mp);
}

void
Re_UnloadMorphPackResource(struct NeMorphPack *mp, NeHandle h)
{
	Sys_Free(mp->morphs);
	Sys_Free(mp->deltas);
}

/* NekoEngine
 *
 * Model.c
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
