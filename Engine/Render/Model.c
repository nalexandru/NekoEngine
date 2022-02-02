#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Render/Model.h>
#include <Render/Render.h>
#include <System/Memory.h>

static inline bool _InitModel(struct NeModel *mdl);

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
		if (!(mdl->cpu.vertexWeights = Sys_Alloc(ci->vertexWeightSize, 1, MH_Render)))
			goto error;
		memcpy(mdl->cpu.vertexWeights, ci->vertexWeights, ci->vertexWeightSize);
		mdl->cpu.vertexWeightSize = ci->vertexWeightSize;
	} else {
		mdl->cpu.vertexWeights = NULL;
		mdl->cpu.vertexWeightSize = 0;
	}

	if (!(mdl->meshes = Sys_Alloc(meshSize, 1, MH_Render)))
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

	return _InitModel(mdl);

error:
	Sys_Free(mdl->cpu.vertices);
	Sys_Free(mdl->cpu.indices);
	Sys_Free(mdl->meshes);

	return false;
}

bool
Re_LoadModelResource(struct NeResourceLoadInfo *li, const char *args, struct NeModel *mdl, NeHandle h)
{
	if (!E_LoadNMeshAsset(&li->stm, mdl))
		return false;

	if (args && strstr(args, "dynamic"))
		mdl->dynamic = true;

	return _InitModel(mdl);
}

void
Re_UnloadModelResource(struct NeModel *mdl, NeHandle h)
{
	for (uint32_t i = 0; i < mdl->meshCount; ++i)
		E_UnloadResource(mdl->meshes[i].materialResource);

	Re_Destroy(mdl->gpu.vertexBuffer);
	Re_Destroy(mdl->gpu.indexBuffer);

	Rt_TermArray(&mdl->skeleton.bones);
	Rt_TermArray(&mdl->skeleton.nodes);

	Sys_Free(mdl->cpu.vertices);
	Sys_Free(mdl->cpu.indices);
	Sys_Free(mdl->meshes);

	if (mdl->cpu.vertexWeights) {
		Re_Destroy(mdl->gpu.vertexWeightBuffer);
		Sys_Free(mdl->cpu.vertexWeights);
	}
}

static inline bool
_InitModel(struct NeModel *mdl)
{
	struct NeBufferCreateInfo bci =
	{
		.desc =
		{
			.size = mdl->cpu.vertexSize,
			.usage = BU_STORAGE_BUFFER | BU_TRANSFER_DST,
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
		bci.desc.size = bci.dataSize = mdl->cpu.vertexWeightSize;
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
