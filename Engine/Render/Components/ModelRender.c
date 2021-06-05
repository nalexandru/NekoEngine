#include <stdlib.h>

#include <System/Memory.h>
#include <Render/Render.h>
#include <Render/Components/ModelRender.h>
#include <Engine/Resource.h>

bool
Re_InitModelRender(struct ModelRender *mr, const void **args)
{
	Handle model = E_INVALID_HANDLE;
	mr->model = E_INVALID_HANDLE;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Model", len)) {
			model = E_LoadResource(*(++args), RES_MODEL);
		} else if (!strncmp(arg, "__ModelHandle", len)) {
			model = (Handle)(*(++args));
		} else if (!strncmp(arg, "Material", len)) {
		}
	}

	// TODO: material override

	if (model != E_INVALID_HANDLE)
		Re_SetModel(mr, model);

	return true;
}

void
Re_SetModel(struct ModelRender *mr, Handle model)
{
	struct Model *new = E_ResourcePtr(model);
	struct Model *old = E_ResourcePtr(mr->model);

	if (old)
		Re_TermModelRender(mr);

	if (!new)
		return;

	mr->model = model;

	mr->materials = Sys_Alloc(new->meshCount, sizeof(*mr->materials), MH_Render);
	mr->meshBounds = Sys_Alloc(new->meshCount, sizeof(*mr->meshBounds), MH_Render);

	for (uint32_t i = 0; i < new->meshCount; ++i)
		Re_InitMaterial(new->meshes[i].materialResource, &mr->materials[i]);
}

void
Re_TermModelRender(struct ModelRender *mr)
{
	struct Model *m = E_ResourcePtr(mr->model);
	if (!m)
		return;

	for (uint32_t i = 0; i < m->meshCount; ++i) {
		Re_TermMaterial(&mr->materials[i]);
		Sys_Free(mr->meshBounds[i]);
	}

	E_UnloadResource(mr->model);
	Sys_Free(mr->meshBounds);
	Sys_Free(mr->materials);
}
