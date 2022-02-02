#include <stdlib.h>

#include <System/Memory.h>

#include "NullGraphicsDriver.h"

struct NeFramebuffer *
NG_CreateFramebuffer(struct NeRenderDevice *dev, const struct NeFramebufferDesc *desc)
{
	struct NeFramebuffer *fb = Sys_Alloc(1, sizeof(*fb), MH_Transient);
	return fb;
}

void
NG_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex)
{
}

void
NG_DestroyFramebuffer(struct NeRenderDevice *dev, struct NeFramebuffer *fb)
{
//	Sys_Free(fb);
}
