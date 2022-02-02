#include <stdlib.h>

#include <System/Memory.h>
#include <Render/Render.h>
#include <Render/Components/ModelRender.h>
#include <Engine/Resource.h>
#include <Scene/Components.h>

static bool _InitModelRender(struct NeModelRender *mr, const void **args);
static void _TermModelRender(struct NeModelRender *mr);

E_REGISTER_COMPONENT(MODEL_RENDER_COMP, struct NeModelRender, 1, _InitModelRender, _TermModelRender)

void
Re_SetModel(struct NeModelRender *mr, NeHandle model)
{
	struct NeModel *new = E_ResourcePtr(model);
	struct NeModel *old = E_ResourcePtr(mr->model);

	if (old)
		_TermModelRender(mr);

	if (!new)
		return;

	mr->model = model;

	mr->materials = Sys_Alloc(new->meshCount, sizeof(*mr->materials), MH_Render);
	mr->meshBounds = Sys_Alloc(new->meshCount, sizeof(*mr->meshBounds), MH_Render);

	for (uint32_t i = 0; i < new->meshCount; ++i)
		Re_InitMaterial(new->meshes[i].materialResource, &mr->materials[i]);

	mr->vertexBuffer = new->gpu.vertexBuffer;
}

static bool
_InitModelRender(struct NeModelRender *mr, const void **args)
{
	NeHandle model = E_INVALID_HANDLE;
	mr->model = E_INVALID_HANDLE;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Model", len)) {
			model = E_LoadResource(*(++args), RES_MODEL);
		} else if (!strncmp(arg, "__ModelHandle", len)) {
			model = (NeHandle)(*(++args));
		} else if (!strncmp(arg, "Material", len)) {
		}
	}

	// TODO: material override

	if (model != E_INVALID_HANDLE)
		Re_SetModel(mr, model);

	return true;
}

static void
_TermModelRender(struct NeModelRender *mr)
{
	struct NeModel *m = E_ResourcePtr(mr->model);
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
