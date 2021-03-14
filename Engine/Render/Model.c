#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Render/Model.h>
#include <Render/Render.h>

bool
Re_CreateModelResource(const char *name, const struct ModelCreateInfo *ci, struct Model *mdl, Handle h)
{
	const size_t vertexSize = sizeof(*mdl->vertices) * ci->vertexCount;
	const size_t indexSize = sizeof(*mdl->indices) * ci->indexCount;
	const size_t meshSize = sizeof(*mdl->meshes) * ci->meshCount;
	
	if (!(mdl->vertices = calloc(1, vertexSize)))
		goto error;
	memcpy(mdl->vertices, ci->vertices, vertexSize);
	mdl->vertexCount = ci->vertexCount;
	
	if (!(mdl->indices = calloc(1, indexSize)))
		goto error;
	memcpy(mdl->indices, ci->indices, indexSize);
	mdl->indexCount = ci->indexCount;
	
	if (!(mdl->meshes = calloc(1, meshSize)))
		goto error;
	memcpy(mdl->meshes, ci->meshes, meshSize);
	mdl->meshCount = ci->meshCount;
	
	// TODO: materials
	
error:
	free(mdl->vertices);
	free(mdl->indices);
	free(mdl->meshes);
	
	return false;
}

bool
Re_LoadModelResource(struct ResourceLoadInfo *li, const char *args, struct Model *mdl, Handle h)
{
	bool rc = false;
	
	if (strstr(li->path, ".glb") || strstr(li->path, ".gltf"))
		rc = E_LoadglTFAsset(li->path, &li->stm, mdl);
	else
		rc = E_LoadNMeshAsset(&li->stm, mdl);
	
	if (!rc)
		return false;
		
	// upload data ?
		
	return true;		
}

void
Re_UnloadModelResource(struct Model *mdl, Handle h)
{
	for (uint32_t i = 0; i < mdl->meshCount; ++i) {
		// destroy material
	}
	
	free(mdl->vertices);
	free(mdl->indices);
	free(mdl->meshes);
}
