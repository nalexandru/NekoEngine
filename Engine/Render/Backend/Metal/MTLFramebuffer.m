#include <stdlib.h>

#include "MTLBackend.h"

struct NeFramebuffer *
Re_CreateFramebuffer(const struct NeFramebufferDesc *desc)
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
Re_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex)
{
	fb->attachments[pos] = tex->tex;
}

void
Re_DestroyFramebuffer(struct NeFramebuffer *fb)
{
	Sys_Free(fb->attachments);
	Sys_Free(fb);
}
