#include <stdlib.h>

#include "MTLDriver.h"

struct Framebuffer *
MTL_CreateFramebuffer(id<MTLDevice> dev, const struct FramebufferDesc *desc)
{
	enum MemoryHeap heap = MH_Frame;
	
	struct Framebuffer *fb = Sys_Alloc(sizeof(*fb), 1, heap);
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
MTL_SetAttachment(struct Framebuffer *fb, uint32_t pos, struct Texture *tex)
{
	fb->attachments[pos] = tex->tex;
}

void
MTL_DestroyFramebuffer(id<MTLDevice> dev, struct Framebuffer *fb)
{
	Sys_Free(fb->attachments);
	Sys_Free(fb);
}
