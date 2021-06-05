#include "MTLDriver.h"

static id<MTLHeap> _heap;

struct Texture *
MTL_CreateTransientTexture(id<MTLDevice> dev, const struct TextureDesc *tDesc, uint16_t location, uint64_t offset)
{
	MTLTextureDescriptor *desc = MTL_TextureDescriptor(dev, tDesc);
	if (!desc)
		return NULL;
	
	struct Texture *tex = Sys_Alloc(sizeof(*tex), 1, MH_Frame);
	if (!tex) {
		[desc release];
		return NULL;
	}
	
	desc.storageMode = MTLStorageModePrivate;
	desc.cpuCacheMode = MTLCPUCacheModeDefaultCache;
	
	tex->tex = [_heap newTextureWithDescriptor: desc offset: offset];
	[desc release];
	
	if (location)
		MTL_SetTexture(location, tex->tex);
	
	return tex;
}

struct Buffer *
MTL_CreateTransientBuffer(id<MTLDevice> dev, const struct BufferDesc *desc, uint16_t location, uint64_t offset)
{
	struct Buffer *buff = Sys_Alloc(sizeof(*buff), 1, MH_Frame);
	if (!buff)
		return NULL;
	
	MTLResourceOptions options = MTL_GPUMemoryTypetoResourceOptions([dev hasUnifiedMemory], desc->memoryType);
	
	buff->buff = [_heap newBufferWithLength: desc->size options: options offset: offset];
	buff->location = location;
	
	if (!buff->buff) {
		Sys_Free(buff);
		return NULL;
	}
	
	MTL_SetBuffer(location, buff->buff);
	buff->memoryType = desc->memoryType;
	
	return buff;
}

bool
MTL_InitTransientHeap(id<MTLDevice> dev, uint64_t size)
{
	MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
	heapDesc.size = size;
	heapDesc.storageMode = MTLStorageModePrivate;
	heapDesc.cpuCacheMode = MTLCPUCacheModeDefaultCache;
	heapDesc.hazardTrackingMode = MTLHazardTrackingModeUntracked;
	heapDesc.type = MTLHeapTypePlacement;
	_heap = [dev newHeapWithDescriptor: heapDesc];
	
	return _heap != nil;
}

bool
MTL_ResizeTransientHeap(id<MTLDevice> dev, uint64_t size)
{
	[_heap release];
	
	MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
	heapDesc.size = size;
	heapDesc.storageMode = MTLStorageModePrivate;
	heapDesc.hazardTrackingMode = MTLHazardTrackingModeUntracked;
	heapDesc.type = MTLHeapTypePlacement;
	_heap = [dev newHeapWithDescriptor: heapDesc];
	
	return _heap != nil;
}

void
MTL_TermTransientHeap(id<MTLDevice> dev)
{
	[_heap release];
}
