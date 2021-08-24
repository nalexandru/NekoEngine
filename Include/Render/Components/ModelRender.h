#ifndef _NE_RENDER_COMPONENTS_MODEL_RENDER_H_
#define _NE_RENDER_COMPONENTS_MODEL_RENDER_H_

#include <Render/Types.h>
#include <Engine/Component.h>

struct ModelRender
{
	COMPONENT_BASE;

	Handle model;
	struct Material *materials;
	uint32_t *bounds, **meshBounds;
};

bool Re_InitModelRender(struct ModelRender *mr, const void **args);
void Re_SetModel(struct ModelRender *mr, Handle model);
void Re_TermModelRender(struct ModelRender *mr);

#endif /* _NE_RENDER_COMPONENTS_MODEL_RENDER_H_ */
