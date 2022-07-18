#include "MTLBackend.h"

struct HeapInfo
{
	id<MTLDevice> dev;
	id<MTLHeap> heap;
	MTLResourceOptions options;
};

static struct NeArray _textureHeapInfo, _bufferHeapInfo, _textureHeaps, _bufferHeaps;

bool
MTLDrv_InitMemory(void)
{
	Rt_InitArray(&_bufferHeapInfo, 10, sizeof(struct HeapInfo), MH_RenderDriver);
	Rt_InitArray(&_textureHeapInfo, 10, sizeof(struct HeapInfo), MH_RenderDriver);

	Rt_InitPtrArray(&_bufferHeaps, 10, MH_RenderDriver);
	Rt_InitPtrArray(&_textureHeaps, 10, MH_RenderDriver);

	return true;
}

id<MTLBuffer>
MTLDrv_CreateBuffer(id<MTLDevice> dev, uint64_t size, MTLResourceOptions options)
{
	struct HeapInfo *info;
	Rt_ArrayForEach(info, &_bufferHeapInfo) {
		if (info->dev != dev || info->options != options)
			continue;

		id<MTLBuffer> buff = [info->heap newBufferWithLength: size options: options];
		if (buff)
			return buff;
	}

	MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
	heapDesc.size = E_GetCVarU32("MetalDrv_BufferHeapSize", 128 * 1024 * 1024)->u32;
	heapDesc.resourceOptions = options;
	heapDesc.hazardTrackingMode = MTLHazardTrackingModeUntracked;
	heapDesc.type = MTLHeapTypeAutomatic;

	id<MTLHeap> heap = [dev newHeapWithDescriptor: heapDesc];

	[heapDesc release];

	struct HeapInfo newInfo = { .dev = dev, .heap = heap, .options = options };
	Rt_ArrayAdd(&_bufferHeapInfo, &newInfo);
	Rt_ArrayAddPtr(&_bufferHeaps, heap);

	return [heap newBufferWithLength: size options: options];
}

id<MTLTexture>
MTLDrv_CreateTexture(id<MTLDevice> dev, MTLTextureDescriptor *desc)
{
	struct HeapInfo *info;
	Rt_ArrayForEach(info, &_textureHeapInfo) {
		if (info->dev != dev || info->options != [desc resourceOptions])
			continue;

		id<MTLTexture> tex = [info->heap newTextureWithDescriptor: desc];
		if (tex)
			return tex;
	}

	MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
	heapDesc.size = E_GetCVarU32("MetalDrv_TextureHeapSize", 256 * 1024 * 1024)->u32;
	heapDesc.resourceOptions = [desc resourceOptions];
	heapDesc.hazardTrackingMode = MTLHazardTrackingModeUntracked;
	heapDesc.type = MTLHeapTypeAutomatic;
	id<MTLHeap> heap = [dev newHeapWithDescriptor: heapDesc];

	struct HeapInfo newInfo = { .dev = dev, .heap = heap, .options = [desc resourceOptions] };
	Rt_ArrayAdd(&_textureHeapInfo, &newInfo);
	Rt_ArrayAddPtr(&_textureHeaps, heap);

	return [heap newTextureWithDescriptor: desc];
}

void
MTLDrv_SetRenderHeaps(id<MTLRenderCommandEncoder> encoder)
{
	if (_textureHeaps.count)
		[encoder useHeaps: (id<MTLHeap> *)_textureHeaps.data count: _textureHeaps.count stages: MTLRenderStageFragment];

	if (_bufferHeaps.count)
		[encoder useHeaps: (id<MTLHeap> *)_bufferHeaps.data count: _bufferHeaps.count stages: MTLRenderStageVertex | MTLRenderStageFragment |
																							  MTLRenderStageObject | MTLRenderStageMesh | MTLRenderStageTile];
}

void
MTLDrv_SetComputeHeaps(id<MTLComputeCommandEncoder> encoder)
{
	if (_textureHeaps.count)
		[encoder useHeaps: (id<MTLHeap> *)_textureHeaps.data count: _textureHeaps.count];

	if (_bufferHeaps.count)
		[encoder useHeaps: (id<MTLHeap> *)_bufferHeaps.data count: _bufferHeaps.count];
}

void
MTLDrv_TermMemory(void)
{
	struct HeapInfo *info;

	Rt_ArrayForEach(info, &_textureHeapInfo)
		[info->heap release];
	Rt_TermArray(&_textureHeapInfo);
	Rt_TermArray(&_textureHeaps);

	Rt_ArrayForEach(info, &_bufferHeapInfo)
		[info->heap release];
	Rt_TermArray(&_bufferHeapInfo);
	Rt_TermArray(&_bufferHeaps);
}
