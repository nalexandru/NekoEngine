#ifndef _RE_MODEL_RENDER_H_
#define _RE_MODEL_RENDER_H_

#include <Engine/Types.h>
#include <Engine/Component.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ModelRender
{
	COMPONENT_BASE;

	Handle model;
	Handle *materials;
};

bool Re_InitModelRender(struct ModelRender *comp, const void **args);
void Re_TermModelRender(struct ModelRender *comp);

#ifdef __cplusplus
}
#endif

#endif /* _RE_MODEL_RENDER_H_ */