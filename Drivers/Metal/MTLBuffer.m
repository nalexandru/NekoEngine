#define Handle __EngineHandle

#include <Render/Device.h>
#include <Render/Buffer.h>

#undef Handle

#include "MTLDriver.h"

struct Buffer *
MTL_CreateBuffer(id<MTLDevice> dev, const struct BufferCreateInfo *bci)
{
	struct Buffer *buff = calloc(1, sizeof(*buff));
	if (!buff)
		return NULL;
	
	MTLResourceOptions options = MTL_GPUMemoryTypetoResourceOptions([dev hasUnifiedMemory], bci->desc.memoryType);
	
	if (bci->data && bci->dataSize == bci->desc.size)
		buff->buff = [dev newBufferWithBytes: bci->data length: bci->desc.size options: options];
	else
		buff->buff = [dev newBufferWithLength: bci->dataSize options: options];
	
	if (!buff->buff) {
		free(buff);
		return NULL;
	}
	
	memcpy(&buff->desc, &bci->desc, sizeof(buff->desc));
	
	return NULL;
}

void
MTL_UpdateBuffer(id<MTLDevice> dev, struct Buffer *buff, uint64_t offset, uint8_t *data, uint64_t size)
{
	uint8_t *dst = [buff->buff contents];
	dst += offset;
	
	memcpy(dst, data, size);
	
#if TARGET_OS_OSX
	if (![dev hasUnifiedMemory])
		[buff->buff didModifyRange: NSMakeRange(offset, size)];
#endif
}

const struct BufferDesc *
MTL_BufferDesc(const struct Buffer *buff)
{
	return &buff->desc;
}

void
MTL_DestroyBuffer(id<MTLDevice> dev, struct Buffer *buff)
{
	[buff->buff release];
	free(buff);
}
