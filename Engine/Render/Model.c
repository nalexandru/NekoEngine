#include <string.h>
#include <stdlib.h>

#include <Render/Model.h>
#include <Render/Material.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <System/System.h>

bool
Re_CreateModel(const char *name, const struct ModelCreateInfo *ci, struct Model *m, Handle h)
{
	size_t i = 0;
	const size_t vertexSize = sizeof(*m->vertices) * ci->vertexCount;
	const size_t indexSize = sizeof(*m->indices) * ci->indexCount;
	const size_t meshSize = sizeof(*m->meshes) * ci->meshCount;

	if (!(m->vertices = calloc(1, vertexSize)))
		goto error;
	memcpy(m->vertices, ci->vertices, vertexSize);
	m->numVertices = ci->vertexCount;

	if (!(m->indices = calloc(1, indexSize)))
		goto error;
	memcpy(m->indices, ci->indices, indexSize);
	m->numIndices = ci->indexCount;

	if (!(m->meshes = calloc(1, meshSize)))
		goto error;
	memcpy(m->meshes, ci->meshes, meshSize);
	m->numMeshes = ci->meshCount;

	if (!(m->materialNames = calloc(1, sizeof(*m->materialNames) * ci->meshCount)))
		goto error;
	if (!(m->materialInstances = calloc(ci->meshCount, sizeof(*m->materialInstances))))
		goto error;

	for (i = 0; i < ci->meshCount; ++i) {
		m->materialNames[i] = wcsdup(ci->materials[i]);
		Re_InstantiateMaterial(m->materialNames[i], &m->materialInstances[i]);
	}

	if (Re_InitModel(name, m))
		return true;

error:
	free(m->vertices);
	free(m->indices);
	free(m->meshes);
	free(m->materialNames);
	free(m->materialInstances);

	return false;
}

bool
Re_LoadModel(struct ResourceLoadInfo *li, const char *args, struct Model *m, Handle h)
{
	size_t i = 0;

	if (strstr(li->path, ".glb") || strstr(li->path, ".gltf")) {
		if (!E_LoadglTFAsset(li->path, &li->stm, m))
			goto error;
	} else {
		if (!E_LoadNMeshAsset(&li->stm, m))
			goto error;
	}

	m->materialInstances = calloc(m->numMeshes, sizeof(*m->materialInstances));
	if (!m->materialInstances)
		goto error;

	for (i = 0; i < m->numMeshes; ++i)
		Re_InstantiateMaterial(m->materialNames[i], &m->materialInstances[i]);

	if (Re_InitModel(li->path, m))
		return true;

error:
	free(m->vertices);
	free(m->indices);
	free(m->meshes);
	free(m->materialNames);
	free(m->materialInstances);

	return false;
}

void
Re_UnloadModel(struct Model *m, Handle h)
{
	size_t i = 0;

	for (i = 0; i < m->numMeshes; ++i) {
		Re_DestroyMaterialInstance(&m->materialInstances[i]);
		free(m->materialNames[i]);
	}

	free(m->vertices);
	free(m->indices);
	free(m->meshes);
	free(m->materialNames);
	free(m->materialInstances);

	Re_TermModel(m);
}
