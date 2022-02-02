#include <stdlib.h>

#include <System/Memory.h>

#include "OpenGLDriver.h"

struct NeFramebuffer *
GL_CreateFramebuffer(struct NeRenderDevice *dev, const struct NeFramebufferDesc *desc)
{
	if (desc->attachmentCount > 9)
		return NULL;

	enum NeMemoryHeap heap = MH_Frame;

	struct NeFramebuffer *fb = Sys_Alloc(1, sizeof(*fb), heap);
	if (!fb)
		return NULL;

	glCreateFramebuffers(1, &fb->id);

	fb->attachmentCount = desc->attachmentCount;
	fb->width = desc->width;
	fb->height = desc->height;
	fb->layers = desc->layers;

//#ifdef _DEBUG
//	if (desc->name)
//		Vkd_SetObjectName(dev->dev, fb->fb, VK_OBJECT_TYPE_FRAMEBUFFER, desc->name);
//#endif

	return fb;
}

void
GL_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex)
{
	fb->attachments[pos] = tex->id;
}

void
GL_DestroyFramebuffer(struct NeRenderDevice *dev, struct NeFramebuffer *fb)
{
	glDeleteFramebuffers(1, &fb->id);
	Sys_Free(fb);
}
