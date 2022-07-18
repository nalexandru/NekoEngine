#include "MTLBackend.h"

id<MTLDevice> MTL_device = nil;

struct NeRenderDevice *
Re_CreateDevice(struct NeRenderDeviceInfo *info)
{
	MTL_device = (id<MTLDevice>)info->private;

	MTL_InitLibrary((id<MTLDevice>)info->private);
	MTL_InitArgumentBuffer((id<MTLDevice>)info->private);

	return info->private;
}

bool
Re_Execute(id<MTLDevice> dev, struct NeRenderContext *ctx, bool wait)
{
	return false;
}

void
Re_WaitIdle(void)
{
	dispatch_semaphore_wait(MTL_frameSemaphore, DISPATCH_TIME_FOREVER);

	id<MTLEvent> event = [MTL_device newEvent];
	uint64_t value = 0;

	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i) {
		id<MTLCommandBuffer> cb = [Re_contexts[i]->queue commandBufferWithUnretainedReferences];
		[cb encodeSignalEvent: event value: ++value];
		[cb commit];
	}

	id<MTLCommandBuffer> cb = [Re_contexts[0]->queue commandBufferWithUnretainedReferences];
	[cb encodeWaitForEvent: event value: value];

	__block dispatch_semaphore_t bds = dispatch_semaphore_create(1);
	[cb addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull cmdBuff) {
		dispatch_semaphore_signal(bds);
	}];
	[cb commit];

	dispatch_semaphore_wait(bds, DISPATCH_TIME_FOREVER);
}

void
Re_DestroyDevice(struct NeRenderDevice *dev)
{
	MTL_TermArgumentBuffer((id<MTLDevice>)dev);
	MTL_TermLibrary();
	[(id<MTLDevice>)dev release];
}
