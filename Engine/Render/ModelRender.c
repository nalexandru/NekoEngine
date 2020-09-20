#include <Render/Model.h>
#include <Render/ModelRender.h>
#include <Engine/Resource.h>
#include <Runtime/Runtime.h>

bool
Re_InitModelRender(struct ModelRender *comp, const void **args)
{
	const char *model = NULL;

	while (args && *args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Model", len))
			model = *(++args);

		++args;
	}
	
	if (model) {
		comp->model = E_LoadResource(model, RES_MODEL);
		if (comp->model == E_INVALID_HANDLE)
			return false;
	} else {
		comp->model = E_INVALID_HANDLE;
	}

	return true;
}

void
Re_TermModelRender(struct ModelRender *comp)
{
	if (comp->model != E_INVALID_HANDLE)
		E_UnloadResource(comp->model);
}
