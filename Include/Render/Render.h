#ifndef _RE_RENDER_H_
#define _RE_RENDER_H_

#include <Engine/Types.h>

extern void *Re_Surface;
extern void *Re_Swapchain;

bool Re_InitRender(void);
void Re_TermRender(void);

#endif /* _RE_RENDER_H_ */
