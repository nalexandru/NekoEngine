#include "MTLDriver.h"

static id<MTLHeap> _heap;

struct NeTexture *
MTL_CreateTransientTexture(id<MTLDevice> dev, const struct NeTextureDesc *tDesc, uint16_t location, uint64_t offset, uint64_t *size)
{
	MTLTextureDescriptor *desc = MTL_TextureDescriptor(dev, tDesc);
	if (!desc)
		return NULL;
	
	struct NeTexture *tex = Sys_Alloc(sizeof(*tex), 1, MH_Frame);
	if (!tex) {
		[desc release];
		return NULL;
	}
	
	desc.storageMode = MTLStorageModePrivate;
	desc.cpuCacheMode = MTLCPUCacheModeWriteCombined;
	
	tex->tex = [_heap newTextureWithDescriptor: desc offset: offset];
	[desc release];

	*size = [tex->tex allocatedSize];
	
	if (location)
		MTL_SetTexture(location, tex->tex);
	
	return tex;
}

struct NeBuffer *
MTL_CreateTransientBuffer(id<MTLDevice> dev, const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeBuffer *buff = Sys_Alloc(sizeof(*buff), 1, MH_Frame);
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

	*size = [buff->buff allocatedSize];
	
	return buff;
}

bool
MTL_InitTransientHeap(id<MTLDevice> dev, uint64_t size)
{
	MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
	heapDesc.size = size;
	heapDesc.storageMode = MTLStorageModePrivate;
	heapDesc.cpuCacheMode = MTLCPUCacheModeWriteCombined;
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
