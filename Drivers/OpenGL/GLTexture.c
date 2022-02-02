#include <stdlib.h>

#include "OpenGLDriver.h"

struct NeTexture *
GL_CreateTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_RenderDriver);
	if (!tex)
		return NULL;

	tex->transient = false;

	switch (desc->type) {
	case TT_2D: tex->target = GL_TEXTURE_2D; break;
	case TT_3D: tex->target = GL_TEXTURE_3D; break;
	case TT_Cube: tex->target = GL_TEXTURE_CUBE_MAP; break;
	case TT_2D_Multisample: tex->target = GL_TEXTURE_2D; break;
	}

	glCreateTextures(tex->target, 1, &tex->id);

	switch (tex->target) {
	case GL_TEXTURE_2D: glTextureStorage2D(tex->id, desc->mipLevels, NeToGLInternalFormat(desc->format), desc->width, desc->height); break;
	case GL_TEXTURE_3D: glTextureStorage3D(tex->id, desc->mipLevels, NeToGLInternalFormat(desc->format), desc->width, desc->height, desc->depth); break;
	}

	tex->addr = glGetTextureHandleARB(tex->id);
	glMakeTextureHandleResidentARB(tex->addr);

	//GL_SetTexture(dev, location, tex->imageView);
	
/*#ifdef _DEBUG
	if (desc->name && tex->memory) {
		size_t tmpLen = strlen(desc->name) + 8;
		char *tmp = Sys_Alloc(sizeof(*tmp), tmpLen, MH_Transient);
		snprintf(tmp, tmpLen, "%s memory", desc->name);

		Vkd_SetObjectName(dev->dev, tex->memory, VK_OBJECT_TYPE_DEVICE_MEMORY, tmp);
	}
#endif*/

	return tex;
	/*
error:
	Sys_Free(tex);
	return NULL;*/
}

enum NeTextureLayout
GL_TextureLayout(const struct NeTexture *tex)
{
//	return VkToNeImageLayout(tex->layout);
	return (enum NeTextureLayout)0;
}

void
GL_DestroyTexture(struct NeRenderDevice *dev, struct NeTexture *tex)
{
	if (!tex->transient) {
		glMakeTextureHandleNonResidentARB(tex->addr);
		glDeleteTextures(1, &tex->id);
	} /*else {
		vkDestroyImageView(dev->dev, tex->imageView, Vkd_transientAllocCb);
		vkDestroyImage(dev->dev, tex->image, Vkd_transientAllocCb);
	}*/

	Sys_Free(tex);
}
