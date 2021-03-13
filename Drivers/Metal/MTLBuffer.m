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
	
	buff->buff = [dev newBufferWithLength: bci->dataSize options: options];
	
	if (!buff->buff) {
		free(buff);
		return NULL;
	}
	
	memcpy(&buff->desc, &bci->desc, sizeof(buff->desc));
	
	if (bci->data)
		MTL_UpdateBuffer(dev, buff, 0, bci->data, bci->dataSize);
	
	return buff;
}

void
MTL_UpdateBuffer(id<MTLDevice> dev, struct Buffer *buff, uint64_t offset, uint8_t *data, uint64_t size)
{
	if (buff->desc.memoryType == MT_GPU_LOCAL) {
		id<MTLBuffer> staging = [dev newBufferWithBytes: data length: size options: MTLResourceStorageModeShared];
		
		id<MTLCommandQueue> queue = [dev newCommandQueue];
		id<MTLCommandBuffer> cmdBuffer = [queue commandBuffer];
		id<MTLBlitCommandEncoder> encoder = [cmdBuffer blitCommandEncoder];
		
		[encoder copyFromBuffer: staging
				   sourceOffset: 0
					   toBuffer: buff->buff
			  destinationOffset: offset
						   size: size];
		[encoder endEncoding];
		
		[cmdBuffer commit];
		[cmdBuffer waitUntilCompleted];
		
		[encoder release];
		[cmdBuffer release];
		[queue release];
		[staging release];
	} else {
		uint8_t *dst = [buff->buff contents];
		dst += offset;
	
		memcpy(dst, data, size);
	
	#if TARGET_OS_OSX
		if (![dev hasUnifiedMemory] && !(buff->buff.resourceOptions & MTLResourceStorageModeShared))
			[buff->buff didModifyRange: NSMakeRange(offset, size)];
	#endif
	}
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
