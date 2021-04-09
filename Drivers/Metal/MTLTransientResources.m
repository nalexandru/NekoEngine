#define Handle __EngineHandle

#include <Render/Device.h>
#include <Render/TransientResources.h>

#undef Handle

#include "MTLDriver.h"

struct Texture *
MTL_CreateTransientTexture(id<MTLDevice> dev, const struct TextureCreateInfo *tci, uint64_t offset)
{
	return MTL_CreateTextureInternal(dev, tci, true, 0);
}

struct Buffer *
MTL_CreateTransientBuffer(id<MTLDevice> dev, const struct BufferCreateInfo *bci, uint64_t offset)
{
	return MTL_CreateBufferInternal(dev, bci, true, 0);
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
