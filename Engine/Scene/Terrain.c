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

#define TR_MOD	L"Terrain"

struct TerrainVertex
{
	float x, y, z;
	float nx, ny, nz;
	float u, v;
	float tu, tv;
};

bool _Generate(struct ModelCreateInfo *mci, const struct TerrainCreateInfo *tci, const struct TextureCreateInfo *mapInfo);
static inline float _SampleHeightmap(const struct TextureCreateInfo *mapInfo, float u, float v);

bool
Scn_CreateTerrain(struct Scene *scn, const struct TerrainCreateInfo *tci)
{
	bool rc = false;
	char *name = NULL;
	size_t len = 0;
	Handle mdl = E_INVALID_HANDLE;
	EntityHandle entity = ES_INVALID_ENTITY;
	struct Stream stm = { 0 };
	struct ModelRender *mr = NULL;
	struct ModelCreateInfo mci = { 0 };
	struct TextureCreateInfo mapInfo = { 0 };

	if (tci->mapFile) {
		if (!E_FileStream(tci->mapFile, IO_READ, &stm)) {
			Sys_LogEntry(TR_MOD, LOG_CRITICAL, L"Cannot open map file %hs", tci->mapFile);
			goto exit;
		}

		if (!E_LoadTGAAsset(&stm, &mapInfo)) {
			Sys_LogEntry(TR_MOD, LOG_CRITICAL, L"%hs is not a TGA file", tci->mapFile);
			goto exit;
		}

		E_CloseStream(&stm);

		if (mapInfo.desc.width != mapInfo.desc.height || mapInfo.desc.format != TF_R8_UNORM) {
			Sys_LogEntry(TR_MOD, LOG_CRITICAL, L"Invalid map file format");
			goto exit;
		}
	}

	entity = E_CreateEntityS(scn, NULL);
	if (!entity) {
		Sys_LogEntry(TR_MOD, LOG_CRITICAL, L"Failed to create entity");
		goto exit;
	}

	E_AddNewComponentS(scn, entity, E_ComponentTypeId(TRANSFORM_COMP), NULL);
	E_AddNewComponentS(scn, entity, E_ComponentTypeId(MODEL_RENDER_COMP), NULL);
	
	mr = E_GetComponentS(scn, entity, E_ComponentTypeId(MODEL_RENDER_COMP));
	if (!mr) {
		Sys_LogEntry(TR_MOD, LOG_CRITICAL, L"Failed to retrieve ModelRender");
		goto exit;
	}

	if (!_Generate(&mci, tci, &mapInfo)) {
		Sys_LogEntry(TR_MOD, LOG_CRITICAL, L"Terrain generation failed");
		goto exit;
	}

	len = wcsnlen(scn->name, sizeof(scn->name) / 2) + 12; // see format string
	name = Sys_Alloc(sizeof(*name), len, MH_Transient);
	if (name)
		snprintf(name, len, "__%ls__Terrain", scn->name);
	else
		name = "__Terrain";

	mdl = E_CreateResource(name, RES_MODEL, &mci);
	if (mdl == E_INVALID_HANDLE) {
		Sys_LogEntry(TR_MOD, LOG_CRITICAL, L"Failed to create model resource");
		goto exit;
	}

	Re_SetModel(mr, mdl);

	rc = true;

exit:
	if (!rc) {
		if (entity)
			E_DestroyEntityS(scn, entity);

		if (mdl)
			E_UnloadResource(mdl);
	}

	Sys_Free(tci->mapFile);
	Sys_Free(mapInfo.data);
	
	return rc;
}

bool
_Generate(struct ModelCreateInfo *mci, const struct TerrainCreateInfo *tci, const struct TextureCreateInfo *mapInfo)
{
	uint32_t *indices, indexCount = 0, vertexCount = 0;
	//struct TerrainVertex *vertices;
	struct Vertex *vertices;
	float uvStep = 1.f / (float)tci->tileCount;
	const float fTileSize = (float)tci->tileSize, halfTiles = (float)tci->tileCount / 2.f;
	
	mci->vertexSize = tci->tileCount * tci->tileCount * 4 * sizeof(*vertices);
	mci->vertices = Sys_Alloc(1, mci->vertexSize, MH_Scene);
	vertices = mci->vertices;
	
	mci->indexType = IT_UINT_32;
	mci->indexSize = tci->tileCount * tci->tileCount * 6 * sizeof(*indices);
	mci->indices = Sys_Alloc(1, mci->indexSize, MH_Scene);
	indices = mci->indices;
	
	// generate geometry
	for (uint32_t i = 0; i <= tci->tileCount; ++i) {
		for (uint32_t j = 0; j <= tci->tileCount; ++j) {
			struct Vertex v;

			v.u = (float)j;
			v.v = (float)i;

			v.x = ((float)j - halfTiles) * fTileSize;
			v.y = _SampleHeightmap(mapInfo, uvStep * j, uvStep * i) * tci->maxHeight;
			v.z = -(halfTiles - i) * fTileSize;

			v.nx = 0.f; v.ny = 0.f; v.nz = 0.f;
			v.tx = 0.f; v.ty = 0.f; v.tz = 0.f;

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
		struct Vertex *vtx0 = &((struct Vertex *)mci->vertices)[indices[i]];
		struct Vertex *vtx1 = &((struct Vertex *)mci->vertices)[indices[i + 1]];
		struct Vertex *vtx2 = &((struct Vertex *)mci->vertices)[indices[i + 2]];
		struct vec3 p0, p1, p2, e1, e2, d;

		v3(&p0, vtx0->x, vtx0->y, vtx0->z);
		v3(&p1, vtx1->x, vtx1->y, vtx1->z);
		v3(&p2, vtx2->x, vtx2->y, vtx2->z);

		v3_sub(&e1, &p1, &p0);
		v3_sub(&e2, &p2, &p0);

		struct vec3 *d0 = (struct vec3 *)&vtx0->nx;
		struct vec3 *d1 = (struct vec3 *)&vtx1->nx;
		struct vec3 *d2 = (struct vec3 *)&vtx2->nx;

		v3_cross(&d, &e1, &e2);
		v3_add(d0, d0, &d);
		v3_add(d1, d2, &d);
		v3_add(d2, d2, &d);

		const float dU1 = vtx1->u - vtx0->u;
		const float dV1 = vtx1->v - vtx0->v;
		const float dU2 = vtx2->u - vtx0->u;
		const float dV2 = vtx2->v - vtx0->v;

		const float f = 1.f / (dU1 * dV2 - dU2 * dV1);

		struct vec3 tgt;
		tgt.x = f * (dV2 * e1.x - dV1 * e2.x);
		tgt.y = f * (dV2 * e1.y - dV1 * e2.y);
		tgt.z = f * (dV2 * e1.z - dV1 * e2.z);

		d0 = (struct vec3 *)&vtx0->tx;
		d1 = (struct vec3 *)&vtx1->tx;
		d2 = (struct vec3 *)&vtx2->tx;

		v3_add(d0, d0, &tgt);
		v3_add(d1, d2, &tgt);
		v3_add(d2, d2, &tgt);
	}

	for (uint32_t i = 0; i < vertexCount; ++i) {
		struct Vertex *v = &((struct Vertex *)mci->vertices)[i];
		struct vec3 *n = (struct vec3 *)&v->nx;
		struct vec3 *t = (struct vec3 *)&v->tx;

		v3_norm(n, n);
		v3_norm(t, t);
	}

	mci->meshes = Sys_Alloc(sizeof(*mci->meshes), 1, MH_Scene);
	mci->meshes[0].vertexCount = vertexCount;
	mci->meshes[0].indexOffset = 0;
	mci->meshes[0].indexCount = indexCount;
	mci->meshes[0].materialResource = tci->material;
	mci->meshCount = 1;
	mci->keepData = false;
	
	return true;
}

static inline float
_SampleHeightmap(const struct TextureCreateInfo *mapInfo, float u, float v)
{
	if (!mapInfo)
		return 0.f;

	uint32_t x = clamp_ui((uint32_t)(u * mapInfo->desc.width), 0, mapInfo->desc.width - 1);
	uint32_t y = clamp_ui((uint32_t)(v * mapInfo->desc.height), 0, mapInfo->desc.height - 1);
	uint32_t val = *((uint8_t *)mapInfo->data + (y * mapInfo->desc.width) + x);

	return (float)val / 255.f;
}
