#include <Engine/Config.h>

#include "NullGraphicsDriver.h"

struct NeTexture *
NG_CreateTransientTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_Frame);
	if (!tex)
		return NULL;

	return tex;
}

struct NeBuffer *
NG_CreateTransientBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeBuffer *buff = Sys_Alloc(1, sizeof(*buff), MH_Frame);
	if (!buff)
		return NULL;

	buff->size = desc->size;
	buff->ptr = NULL;

	return buff;
}

bool
NG_InitTransientHeap(struct NeRenderDevice *dev, uint64_t size)
{
	return true;
}

bool
NG_ResizeTransientHeap(struct NeRenderDevice *dev, uint64_t size)
{
	return false;
}

void
NG_TermTransientHeap(struct NeRenderDevice *dev)
{
}
