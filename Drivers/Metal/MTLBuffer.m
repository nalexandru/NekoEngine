#define Handle __EngineHandle

#include <Render/Device.h>
#include <Render/Buffer.h>

#undef Handle

#include "MTLDriver.h"

struct Buffer *
MTL_CreateBufferInternal(id<MTLDevice> dev, const struct BufferCreateInfo *bci, bool transient, uint16_t location)
{
	struct Buffer *buff = calloc(1, sizeof(*buff));
	if (!buff)
		return NULL;
	
	MTLResourceOptions options = MTL_GPUMemoryTypetoResourceOptions([dev hasUnifiedMemory], bci->desc.memoryType);
	
	if (transient)
		options |= MTLResourceStorageModeMemoryless;
	
	buff->buff = [dev newBufferWithLength: bci->desc.size options: options];
	
	if (!buff->buff) {
		free(buff);
		return NULL;
	}
	
	MTL_SetBuffer(location, buff->buff);
	buff->memoryType = bci->desc.memoryType;
	
	if (bci->data)
		MTL_UpdateBuffer(dev, buff, 0, bci->data, bci->dataSize);
	
	return buff;
}

struct Buffer *
MTL_CreateBuffer(id<MTLDevice> dev, const struct BufferCreateInfo *bci, uint16_t location)
{
	return MTL_CreateBufferInternal(dev, bci, false, location);
}

void
MTL_UpdateBuffer(id<MTLDevice> dev, struct Buffer *buff, uint64_t offset, uint8_t *data, uint64_t size)
{
	if (buff->memoryType == MT_GPU_LOCAL) {
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
		
		[encoder autorelease];
		[cmdBuffer autorelease];
		[queue autorelease];
		[staging autorelease];
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

void
MTL_DestroyBuffer(id<MTLDevice> dev, struct Buffer *buff)
{
	MTL_RemoveBuffer(buff->buff);
	[buff->buff autorelease];
	free(buff);
}
