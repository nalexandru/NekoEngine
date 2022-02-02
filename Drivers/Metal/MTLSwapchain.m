#include <System/Memory.h>

#include "MTLDriver.h"

static NSAutoreleasePool *_pool;
dispatch_semaphore_t MTL_frameSemaphore;

#if TARGET_OS_OSX

void *
MTL_CreateSurface(id<MTLDevice> dev, NSWindow *window)
{
	[window contentView].wantsLayer = true;
	return [window contentView];
}

void
MTL_DestroySurface(id<MTLDevice> dev, NSView *view)
{
	view.wantsLayer = false;
}

struct NeSwapchain *
MTL_CreateSwapchain(id<MTLDevice> dev, VIEWTYPE *view, bool verticalSync)
{
	struct NeSwapchain *sw = Sys_Alloc(1, sizeof(*sw), MH_RenderDriver);
	if (!sw)
		return NULL;

	MTL_frameSemaphore = dispatch_semaphore_create(RE_NUM_FRAMES);
	sw->event = [dev newEvent];
	sw->value = 0;

	CAMetalLayer *layer = (CAMetalLayer *)[view layer];
	[layer setDevice: dev];
	[layer setDisplaySyncEnabled: verticalSync];
	sw->layer = (CAMetalLayer *)[view layer];

	return sw;
}

#else

void *
MTL_CreateSurface(id<MTLDevice> dev, UIWindow *window)
{
	if (![[[window rootViewController] view] isKindOfClass: [MTKView class]])
		return NULL;
	
	MTKView *v = (MTKView *)[[window rootViewController] view];
	[v setDevice: dev];
	
	return [[window rootViewController] view];
}

void
MTL_DestroySurface(id<MTLDevice> dev, UIView *view)
{
	(void)dev;
	(void)view;
}

struct NeSwapchain *
MTL_CreateSwapchain(id<MTLDevice> dev, VIEWTYPE *view, bool verticalSync)
{
	struct NeSwapchain *sw = Sys_Alloc(1, sizeof(*sw), MH_RenderDriver);
	if (!sw)
		return NULL;

	MTL_frameSemaphore = dispatch_semaphore_create(RE_NUM_FRAMES);
	sw->event = [dev newEvent];
	sw->value = 0;

	CAMetalLayer *layer = (CAMetalLayer *)[view layer];
	[layer setDevice: dev];
	sw->layer = (CAMetalLayer *)[view layer];

	return sw;
}

#endif

void
MTL_DestroySwapchain(id<MTLDevice> dev, struct NeSwapchain *sw)
{
	(void)dev;

	[sw->event release];
	Sys_Free(sw);
}

void *
MTL_AcquireNextImage(id<MTLDevice> dev, struct NeSwapchain *sw)
{
	dispatch_semaphore_wait(MTL_frameSemaphore, DISPATCH_TIME_FOREVER);

	_pool = [[NSAutoreleasePool alloc] init];
	return [sw->layer nextDrawable];
}

bool
MTL_Present(id<MTLDevice> dev, struct NeRenderContext *ctx, struct NeSwapchain *sw, id<CAMetalDrawable> image, id<MTLFence> f)
{
	ctx->cmdBuffer = [ctx->queue commandBufferWithUnretainedReferences];

	__block dispatch_semaphore_t blockSemaphore = MTL_frameSemaphore;
	[ctx->cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
		dispatch_semaphore_signal(blockSemaphore);
	}];

	struct Mtld_SubmitInfo *si;
	Rt_ArrayForEach(si, &ctx->submitted.graphics) {
		if (si->signal)
			[si->cmdBuffer encodeSignalEvent: si->signal value: si->signalValue];
		else
			[si->cmdBuffer encodeSignalEvent: sw->event value: ++sw->value];

		[si->cmdBuffer commit];
	}

	[ctx->cmdBuffer encodeWaitForEvent: sw->event value: sw->value];
	[ctx->cmdBuffer presentDrawable: image];
	[ctx->cmdBuffer commit];

	[_pool drain];
	
	return true;
}

struct NeTexture *
MTL_SwapchainTexture(struct NeSwapchain *sw, id<CAMetalDrawable> image)
{
	struct NeTexture *t = Sys_Alloc(1, sizeof(*t), MH_Transient);
	t->tex = [image texture];
	return t;
}

enum NeTextureFormat
MTL_SwapchainFormat(struct NeSwapchain *sw)
{
	return MTLToNeTextureFormat([sw->layer pixelFormat]);
}

void
MTL_SwapchainDesc(struct NeSwapchain *sw, struct NeFramebufferAttachmentDesc *desc)
{
	desc->format = MTLToNeTextureFormat([sw->layer pixelFormat]);
	desc->usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST;
}

void
MTL_ScreenResized(id<MTLDevice> dev, struct NeSwapchain *sw)
{
	(void)dev;
	(void)sw;
}
