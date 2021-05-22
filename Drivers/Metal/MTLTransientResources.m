#include "MTLDriver.h"

struct Texture *
MTL_CreateTransientTexture(id<MTLDevice> dev, const struct TextureCreateInfo *tci, uint64_t offset)
{
	struct Texture *tex = Sys_Alloc(sizeof(*tex), 1, MH_Frame);
	if (!tex)
		return NULL;
	return MTL_CreateTextureInternal(dev, tex, tci, true, 0);
}

struct Buffer *
MTL_CreateTransientBuffer(id<MTLDevice> dev, const struct BufferCreateInfo *bci, uint64_t offset)
{
	struct Buffer *buff = Sys_Alloc(sizeof(*buff), 1, MH_Frame);
	if (!buff)
		return NULL;
	return MTL_CreateBufferInternal(dev, buff, bci, true, 0);
}

bool
MTL_InitTransientHeap(id<MTLDevice> dev, uint64_t size)
{
	return true; // TODO
}

bool
MTL_ResizeTransientHeap(id<MTLDevice> dev, uint64_t size)
{
	return true; // TODO
}

void
MTL_TermTransientHeap(id<MTLDevice> dev)
{
	
}
