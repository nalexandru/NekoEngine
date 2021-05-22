#include "MTLDriver.h"

struct Buffer *
MTL_CreateBufferInternal(id<MTLDevice> dev, struct Buffer *buff, const struct BufferCreateInfo *bci, bool transient, uint16_t location)
{
	MTLResourceOptions options = MTL_GPUMemoryTypetoResourceOptions([dev hasUnifiedMemory], bci->desc.memoryType);
	
	if (transient)
		options |= MTLResourceStorageModeMemoryless;
	
	buff->buff = [dev newBufferWithLength: bci->desc.size options: options];
	
	if (!buff->buff) {
		Sys_Free(buff);
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
	struct Buffer *buff = Sys_Alloc(sizeof(*buff), 1, MH_RenderDriver);
	if (!buff)
		return NULL;
	return MTL_CreateBufferInternal(dev, buff, bci, false, location);
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
		
	#if TARGET_OS_OSX
		[encoder autorelease];
		[cmdBuffer autorelease];
		[queue autorelease];
		[staging autorelease];
	#endif
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

void *
MTL_MapBuffer(id<MTLDevice> dev, struct Buffer *buff)
{
	return [buff->buff contents];
}

void
MTL_UnmapBuffer(id<MTLDevice> dev, struct Buffer *buff)
{
	if (![dev hasUnifiedMemory] && !(buff->buff.resourceOptions & MTLResourceStorageModeShared))
		[buff->buff didModifyRange: NSMakeRange(0, 0)];
	
	(void)dev; (void)buff;
}

uint64_t
MTL_BufferAddress(id<MTLDevice> dev, const struct Buffer *buff, uint64_t offset)
{
	uint64_t r = 0;
	uint32_t *v = (uint32_t *)&r;
	v[0] = buff->location;
	v[1] = (uint32_t)offset;
	return r;
}

void
MTL_DestroyBuffer(id<MTLDevice> dev, struct Buffer *buff)
{
	MTL_RemoveBuffer(buff->buff);
	[buff->buff autorelease];
	Sys_Free(buff);
}
