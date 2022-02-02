#include <stdlib.h>

#include "MTLDriver.h"

struct NeFramebuffer *
MTL_CreateFramebuffer(id<MTLDevice> dev, const struct NeFramebufferDesc *desc)
{
	enum NeMemoryHeap heap = MH_Frame;
	
	struct NeFramebuffer *fb = Sys_Alloc(sizeof(*fb), 1, heap);
	if (!fb)
		return NULL;
	
	fb->attachments = Sys_Alloc(sizeof(*fb->attachments), desc->attachmentCount, heap);
	if (!fb->attachments) {
		Sys_Free(fb);
		return NULL;
	}
	
	fb->attachmentCount = desc->attachmentCount;
	
	return fb;
}

void
MTL_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex)
{
	fb->attachments[pos] = tex->tex;
}

void
MTL_DestroyFramebuffer(id<MTLDevice> dev, struct NeFramebuffer *fb)
{
	Sys_Free(fb->attachments);
	Sys_Free(fb);
}
