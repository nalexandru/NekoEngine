#ifndef _NE_RENDER_COMPONENTS_MODEL_RENDER_H_
#define _NE_RENDER_COMPONENTS_MODEL_RENDER_H_

#include <Render/Types.h>
#include <Engine/Component.h>

struct NeModelRender
{
	NE_COMPONENT_BASE;

	NeHandle model;
	NeBufferHandle vertexBuffer;
	struct NeMaterial *materials;
	uint32_t *bounds, **meshBounds;
};

void Re_SetModel(struct NeModelRender *mr, NeHandle model);

#endif /* _NE_RENDER_COMPONENTS_MODEL_RENDER_H_ */
