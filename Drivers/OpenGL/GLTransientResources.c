#include <Engine/Config.h>

#include "OpenGLDriver.h"

#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

struct NeTexture *
GL_CreateTransientTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_Frame);
	if (!tex)
		return NULL;

	// https://github.com/KhronosGroup/OpenGL-Registry/blob/main/extensions/EXT/EXT_external_objects.txt

//	if (location)
//			GL_SetTexture(dev, location, tex->imageView);

	return tex;
}

struct NeBuffer *
GL_CreateTransientBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeBuffer *buff = Sys_Alloc(1, sizeof(*buff), MH_Frame);
	if (!buff)
		return NULL;

/*#ifdef _DEBUG
	if (desc->name)
		Vkd_SetObjectName(dev->dev, buff->buff, VK_OBJECT_TYPE_BUFFER, desc->name);
#endif*/

	return buff;
}

bool
GL_InitTransientHeap(struct NeRenderDevice *dev, uint64_t size)
{
	return true;
}

bool
GL_ResizeTransientHeap(struct NeRenderDevice *dev, uint64_t size)
{
	return false;
}

void
GL_TermTransientHeap(struct NeRenderDevice *dev)
{
}
