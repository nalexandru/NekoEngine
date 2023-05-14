#include <Math/Math.h>
#include <Runtime/Runtime.h>
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Entity.h>
#include <Engine/Resource.h>
#include <Engine/Component.h>
#include <Scene/Components.h>
#include <Scene/Scene.h>
#include <Render/Render.h>
#include <Render/Components/ModelRender.h>
#include <System/Log.h>

#define TR_MOD	"Terrain"

struct NeTerrainVertex
{
	float x, y, z;
	float nx, ny, nz;
	float u, v;
	float tu, tv;
};

bool Generate(struct NeModelCreateInfo *mci, const struct NeTerrainCreateInfo *tci, const struct NeTextureCreateInfo *mapInfo);
static inline float SampleHeightmap(const struct NeTextureCreateInfo *mapInfo, float u, float v);

bool
Scn_CreateTerrain(struct NeScene *scn, const struct NeTerrainCreateInfo *tci)
{
	bool rc = false;
	char *name = NULL;
	size_t len;
	NeHandle mdl = NE_INVALID_HANDLE;
	NeEntityHandle entity = ES_INVALID_ENTITY;
	struct NeStream stm = { 0 };
	struct NeModelRender *mr = NULL;
	struct NeModelCreateInfo mci = { 0 };
	struct NeTextureCreateInfo mapInfo = { 0 };

	if (tci->mapFile) {
		if (!E_FileStream(tci->mapFile, IO_READ, &stm)) {
			Sys_LogEntry(TR_MOD, LOG_CRITICAL, "Cannot open map file %s", tci->mapFile);
			goto exit;
		}

		if (!Asset_LoadTGA(&stm, &mapInfo)) {
			Sys_LogEntry(TR_MOD, LOG_CRITICAL, "%s is not a TGA file", tci->mapFile);
			goto exit;
		}

		E_CloseStream(&stm);

		if (mapInfo.desc.width != mapInfo.desc.height || mapInfo.desc.format != TF_R8_UNORM) {
			Sys_LogEntry(TR_MOD, LOG_CRITICAL, "Invalid map file format");
			goto exit;
		}
	}

	entity = E_CreateEntityS(scn, "Terrain", NULL);
	if (!entity) {
		Sys_LogEntry(TR_MOD, LOG_CRITICAL, "Failed to create entity");
		goto exit;
	}

	E_AddNewComponent(entity, NE_TRANSFORM_ID, NULL);
	E_AddNewComponent(entity, NE_MODEL_RENDER_ID, NULL);

	mr = (struct NeModelRender *)E_GetComponent(entity, NE_MODEL_RENDER_ID);
	if (!mr) {
		Sys_LogEntry(TR_MOD, LOG_CRITICAL, "Failed to retrieve ModelRender");
		goto exit;
	}

	if (!Generate(&mci, tci, &mapInfo)) {
		Sys_LogEntry(TR_MOD, LOG_CRITICAL, "Terrain generation failed");
		goto exit;
	}

	len = strnlen(scn->name, sizeof(scn->name)) + 12; // see format string
	name = (char *)Sys_Alloc(sizeof(*name), len, MH_Transient);
	if (name)
		snprintf(name, len, "__%s__Terrain", scn->name);
	else
		name = (char *)"__Terrain";

	mdl = E_CreateResource(name, RES_MODEL, &mci);
	if (mdl == NE_INVALID_HANDLE) {
		Sys_LogEntry(TR_MOD, LOG_CRITICAL, "Failed to create model resource");
		goto exit;
	}

	Re_SetModel(mr, mdl);

	rc = true;

exit:
	if (!rc) {
		if (entity)
			E_DestroyEntity(entity);

		if (mdl)
			E_UnloadResource(mdl);
	}

	Sys_Free(tci->mapFile);
	Sys_Free(mapInfo.data);
	
	return rc;
}

bool
Generate(struct NeModelCreateInfo *mci, const struct NeTerrainCreateInfo *tci, const struct NeTextureCreateInfo *mapInfo)
{
	uint32_t *indices, indexCount = 0, vertexCount = 0;
	struct NeVertex *vertices;
	float uvStep = 1.f / (float)tci->tileCount;
	const float fTileSize = (float)tci->tileSize, halfTiles = (float)tci->tileCount / 2.f;
	
	mci->vertexSize = tci->tileCount * tci->tileCount * 4 * sizeof(*vertices);
	mci->vertices = Sys_Alloc(1, mci->vertexSize, MH_Scene);
	vertices = (struct NeVertex *)mci->vertices;
	
	mci->indexType = IT_UINT_32;
	mci->indexSize = tci->tileCount * tci->tileCount * 6 * sizeof(*indices);
	mci->indices = Sys_Alloc(1, mci->indexSize, MH_Scene);
	indices = (uint32_t *)mci->indices;
	
	// generate geometry
	for (uint32_t i = 0; i <= tci->tileCount; ++i) {
		for (uint32_t j = 0; j <= tci->tileCount; ++j) {
			struct NeVertex v;

			v.u = (float)j;
			v.v = (float)i;

			v.x = ((float)j - halfTiles) * fTileSize;
			v.y = SampleHeightmap(mapInfo, uvStep * j, uvStep * i) * tci->maxHeight;
			v.z = -(halfTiles - i) * fTileSize;

			v.nx = 0.f; v.ny = 0.f; v.nz = 0.f;
			v.tx = 0.f; v.ty = 0.f; v.tz = 0.f;
			v.r = 1.f; v.g = 1.f; v.b = 1.f; v.a = 1.f;

			*vertices++ = v;
			++vertexCount;
		
			if (i < tci->tileCount && j < tci->tileCount) {
				int idxOffset = j + i * (tci->tileCount + 1);
				indices[indexCount++] = idxOffset + tci->tileCount + 1;
				indices[indexCount++] = idxOffset + 1;
				indices[indexCount++] = idxOffset;

				++idxOffset;
				indices[indexCount++] = idxOffset + tci->tileCount;
				indices[indexCount++] = idxOffset + tci->tileCount + 1;
				indices[indexCount++] = idxOffset;
			}
		}
	}
	
	// calculate tangents + normals
	for (uint32_t i = 0; i < indexCount; i += 3) {
		struct NeVertex *vtx0 = &((struct NeVertex *)mci->vertices)[indices[i]];
		struct NeVertex *vtx1 = &((struct NeVertex *)mci->vertices)[indices[i + 1]];
		struct NeVertex *vtx2 = &((struct NeVertex *)mci->vertices)[indices[i + 2]];

		const XMVECTOR p0 = XMVectorSet(vtx0->x, vtx0->y, vtx0->z, 1.f);
		const XMVECTOR p1 = XMVectorSet(vtx1->x, vtx1->y, vtx1->z, 1.f);
		const XMVECTOR p2 = XMVectorSet(vtx2->x, vtx2->y, vtx2->z, 1.f);

		const XMVECTOR e1 = XMVectorSubtract(p1, p0);
		const XMVECTOR e2 = XMVectorSubtract(p2, p0);

		const XMVECTOR d = XMVector3Cross(e1, e2);
		const XMVECTOR d0 = XMVectorAdd(XMVectorSet(vtx0->nx, vtx0->ny, vtx0->nz, 1.f), d);
		const XMVECTOR d1 = XMVectorAdd(XMVectorSet(vtx1->nx, vtx1->ny, vtx1->nz, 1.f), d);
		const XMVECTOR d2 = XMVectorAdd(XMVectorSet(vtx2->nx, vtx2->ny, vtx2->nz, 1.f), d);

		vtx0->nx = XMVectorGetX(d0); vtx0->ny = XMVectorGetY(d0); vtx0->nz = XMVectorGetZ(d0);
		vtx1->nx = XMVectorGetX(d1); vtx1->ny = XMVectorGetY(d1); vtx1->nz = XMVectorGetZ(d1);
		vtx2->nx = XMVectorGetX(d2); vtx2->ny = XMVectorGetY(d2); vtx2->nz = XMVectorGetZ(d2);

		const float dU1 = vtx1->u - vtx0->u;
		const float dV1 = vtx1->v - vtx0->v;
		const float dU2 = vtx2->u - vtx0->u;
		const float dV2 = vtx2->v - vtx0->v;

		const float f = 1.f / (dU1 * dV2 - dU2 * dV1);
		const XMVECTOR t = XMVectorSet(
			f * (dV2 * XMVectorGetX(e1) - dV1 * XMVectorGetX(e2)),
			f * (dV2 * XMVectorGetY(e1) - dV1 * XMVectorGetY(e2)),
			f * (dV2 * XMVectorGetZ(e1) - dV1 * XMVectorGetZ(e2)),
			1.f
		);
		const XMVECTOR t0 = XMVectorAdd(XMVectorSet(vtx0->tx, vtx0->ty, vtx0->tz, 1.f), t);
		const XMVECTOR t1 = XMVectorAdd(XMVectorSet(vtx1->tx, vtx1->ty, vtx1->tz, 1.f), t);
		const XMVECTOR t2 = XMVectorAdd(XMVectorSet(vtx2->tx, vtx2->ty, vtx2->tz, 1.f), t);

		vtx0->tx = XMVectorGetX(t0); vtx0->ty = XMVectorGetY(t0); vtx0->tz = XMVectorGetZ(t0);
		vtx1->tx = XMVectorGetX(t1); vtx1->ty = XMVectorGetY(t1); vtx1->tz = XMVectorGetZ(t1);
		vtx2->tx = XMVectorGetX(t2); vtx2->ty = XMVectorGetY(t2); vtx2->tz = XMVectorGetZ(t2);
	}

	for (uint32_t i = 0; i < vertexCount; ++i) {
		struct NeVertex *v = &((struct NeVertex *)mci->vertices)[i];

		const XMVECTOR n = XMVector3Normalize(XMVectorSet(v->nx, v->ny, v->nz, 1.f));
		v->nx = XMVectorGetX(n); v->ny = XMVectorGetY(n); v->nz = XMVectorGetZ(n);

		const XMVECTOR t = XMVector3Normalize(XMVectorSet(v->tx, v->ty, v->tz, 1.f));
		v->tx = XMVectorGetX(t); v->ty = XMVectorGetY(t); v->tz = XMVectorGetZ(t);
	}

	mci->meshes = (struct NeMesh *)Sys_Alloc(sizeof(*mci->meshes), 1, MH_Scene);
	mci->meshes[0].vertexCount = vertexCount;
	mci->meshes[0].indexOffset = 0;
	mci->meshes[0].indexCount = indexCount;
	mci->meshes[0].materialResource = tci->material;
	mci->meshCount = 1;
	mci->keepData = false;
	
	return true;
}

static inline float
SampleHeightmap(const struct NeTextureCreateInfo *mapInfo, float u, float v)
{
	if (!mapInfo)
		return 0.f;

	uint32_t x = M_Clamp((uint32_t)(u * mapInfo->desc.width), (uint32_t)0, mapInfo->desc.width - 1);
	uint32_t y = M_Clamp((uint32_t)(v * mapInfo->desc.height), (uint32_t)0, mapInfo->desc.height - 1);
	uint32_t val = *((uint8_t *)mapInfo->data + ((uint64_t)y * mapInfo->desc.width) + x);

	return (float)val / 255.f;
}

/* NekoEngine
 *
 * Terrain.c
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
