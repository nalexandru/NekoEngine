#define Handle __EngineHandle

#include <stdlib.h>

#include <Render/Device.h>
#include <Render/Framebuffer.h>

#undef Handle

#include "MTLDriver.h"

struct Framebuffer *
MTL_CreateFramebuffer(id<MTLDevice> dev, const struct FramebufferDesc *desc)
{
	struct Framebuffer *fb = malloc(sizeof(*fb));
	if (!fb)
		return NULL;
	
	fb->attachments = calloc(desc->attachmentCount, sizeof(*fb->attachments));
	if (!fb->attachments) {
		free(fb);
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
	free(fb->attachments);
	free(fb);
}
