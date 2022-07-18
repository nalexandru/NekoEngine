#include "MTLBackend.h"

struct NeBuffer *
Re_BkCreateBuffer(const struct NeBufferDesc *desc, uint16_t location)
{
	struct NeBuffer *buff = Sys_Alloc(sizeof(*buff), 1, MH_RenderDriver);
	if (!buff)
		return NULL;
	
	MTLResourceOptions options = MTL_GPUMemoryTypetoResourceOptions([MTL_device hasUnifiedMemory], desc->memoryType);

	if (desc->usage > (BU_TRANSFER_SRC | BU_TRANSFER_DST))
		buff->buff = MTLDrv_CreateBuffer(MTL_device, desc->size, options);
	else
		buff->buff = [MTL_device newBufferWithLength: desc->size options: options];

	if (!buff->buff) {
		Sys_Free(buff);
		return NULL;
	}
	
	MTL_SetBuffer(location, buff->buff);
	buff->memoryType = desc->memoryType;
	buff->location = location;

	return buff;
}

void
Re_BkUpdateBuffer(struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size)
{
	if (buff->memoryType == MT_GPU_LOCAL) {
		id<MTLBuffer> staging = [MTL_device newBufferWithBytes: data length: size options: MTLResourceStorageModeShared];
		
		id<MTLCommandQueue> queue = [MTL_device newCommandQueue];
		id<MTLCommandBuffer> cmdBuffer = [queue commandBufferWithUnretainedReferences];
		id<MTLBlitCommandEncoder> encoder = [cmdBuffer blitCommandEncoder];
		
		[encoder copyFromBuffer: staging
				   sourceOffset: 0
					   toBuffer: buff->buff
			  destinationOffset: offset
						   size: size];
		[encoder endEncoding];
		
		[cmdBuffer commit];
		[cmdBuffer waitUntilCompleted];

		[staging release];
		[queue release];
	} else {
		uint8_t *dst = [buff->buff contents];
		dst += offset;
	
		memcpy(dst, data, size);
	
	#if TARGET_OS_OSX
		if (![MTL_device hasUnifiedMemory] && !(buff->buff.resourceOptions & MTLResourceStorageModeShared))
			[buff->buff didModifyRange: NSMakeRange(offset, size)];
	#endif
	}
}

void *
Re_BkMapBuffer(struct NeBuffer *buff)
{
	return [buff->buff contents];
}

void
Re_BkFlushBuffer(struct NeBuffer *buff, uint64_t offset, uint64_t size)
{
#if TARGET_OS_OSX
	if (![MTL_device hasUnifiedMemory] && !(buff->buff.resourceOptions & MTLResourceStorageModeShared))
		[buff->buff didModifyRange: NSMakeRange(offset, size)];
#else
	(void)buff; (void)offset; (void)size;
#endif
}

void
Re_BkUnmapBuffer(struct NeBuffer *buff)
{
#if TARGET_OS_OSX
	if (![MTL_device hasUnifiedMemory] && !(buff->buff.resourceOptions & MTLResourceStorageModeShared))
		[buff->buff didModifyRange: NSMakeRange(0, 0)];
#else
	(void)buff;
#endif
}

uint64_t
Re_BkBufferAddress(const struct NeBuffer *buff, uint64_t offset)
{
	uint64_t r = 0;
	uint32_t *v = (uint32_t *)&r;
	v[0] = buff->location;
	v[1] = (uint32_t)offset;
	return r;
}

uint64_t
Re_OffsetAddress(uint64_t addr, uint64_t offset)
{
	uint32_t *v = (uint32_t *)&addr;
	v[1] += (uint32_t)offset;
	return addr;
}

void
Re_BkDestroyBuffer(struct NeBuffer *buff)
{
	MTL_RemoveBuffer(buff->buff);
	[buff->buff release];
	Sys_Free(buff);
}
