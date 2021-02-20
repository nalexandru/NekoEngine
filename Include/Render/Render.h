#ifndef _RE_RENDER_H_
#define _RE_RENDER_H_

#include <Engine/Types.h>

#define RE_NUM_FRAMES	3

ENGINE_API extern uint32_t Re_frameId;
ENGINE_API extern void *Re_surface;

bool Re_InitRender(void);
void Re_RenderFrame(void);
void Re_TermRender(void);

#endif /* _RE_RENDER_H_ */
