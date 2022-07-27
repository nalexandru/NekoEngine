#include <System/Memory.h>

#include "MTLBackend.h"

static NSAutoreleasePool *_pool;
dispatch_semaphore_t MTL_frameSemaphore;

#if TARGET_OS_OSX

struct NeSurface *
Re_CreateSurface(void *window)
{
	[(NSWindow *)window contentView].wantsLayer = true;
	return (struct NeSurface *)[(NSWindow *)window contentView];
}

void
Re_DestroySurface(struct NeSurface *surface)
{
	NSView *view = (NSView *)surface;
	view.wantsLayer = false;
}

#else

struct NeSurface *
Re_CreateSurface(void *window)
{
	if (![[[(UIWindow *)window rootViewController] view] isKindOfClass: [MTKView class]])
		return NULL;
	
	MTKView *v = (MTKView *)[[(UIWindow *)window rootViewController] view];
	[v setDevice: MTL_device];
	
	return (struct NeSurface *)[[(UIWindow *)window rootViewController] view];
}

void
Re_DestroySurface(struct NeSurface *surface)
{
	(void)surface;
}

#endif

struct NeSwapchain *
Re_CreateSwapchain(struct NeSurface *surface, bool verticalSync)
{
	struct NeSwapchain *sw = Sys_Alloc(1, sizeof(*sw), MH_RenderDriver);
	if (!sw)
		return NULL;

	MTL_frameSemaphore = dispatch_semaphore_create(1);
	sw->event = [MTL_device newEvent];
	sw->value = 0;

	CAMetalLayer *layer = (CAMetalLayer *)[(VIEWTYPE *)surface layer];
	[layer setDevice: MTL_device];

#if TARGET_OS_OSX
	[layer setDisplaySyncEnabled: verticalSync];
#endif

	sw->layer = (CAMetalLayer *)[(VIEWTYPE *)surface layer];

	return sw;
}

void
Re_DestroySwapchain(struct NeSwapchain *sw)
{
	[sw->event release];
	Sys_Free(sw);
}

void *
Re_AcquireNextImage(struct NeSwapchain *sw)
{
	dispatch_semaphore_wait(MTL_frameSemaphore, DISPATCH_TIME_FOREVER);

	_pool = [[NSAutoreleasePool alloc] init];
	return [sw->layer nextDrawable];
}

bool
Re_Present(struct NeSwapchain *sw, void *image, struct NeSemaphore *waitSemaphore)
{
	// id<CAMetalDrawable> image, id<MTLFence> waitSemaphore
	struct NeRenderContext *ctx = Re_CurrentContext();

	ctx->cmdBuffer = [ctx->queue commandBufferWithUnretainedReferences];

	__block dispatch_semaphore_t blockSemaphore = MTL_frameSemaphore;
	[ctx->cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
		dispatch_semaphore_signal(blockSemaphore);
	}];

	struct Mtld_SubmitInfo *si;
	Rt_ArrayForEach(si, &ctx->submitted.graphics) {
		//if (si->wait)
		//	[si->cmdBuffer encodeWaitForEvent: si->wait value: si->waitValue];
		//else
		if (!si->wait)
			[si->cmdBuffer encodeWaitForEvent: sw->event value: sw->value];

		if (si->signal)
			[si->cmdBuffer encodeSignalEvent: si->signal value: si->signalValue];
		else
			[si->cmdBuffer encodeSignalEvent: sw->event value: ++sw->value];

		[si->cmdBuffer commit];
	}

	/*Rt_ArrayForEach(si, &ctx->submitted.compute) {
		if (si->signal)
			[si->cmdBuffer encodeSignalEvent: si->signal value: si->signalValue];
		else
			[si->cmdBuffer encodeSignalEvent: sw->event value: ++sw->value];

		[si->cmdBuffer commit];
	}*/

	[ctx->cmdBuffer encodeWaitForEvent: sw->event value: sw->value];
	[ctx->cmdBuffer presentDrawable: image];
	[ctx->cmdBuffer commit];

	[_pool drain];
	
	return true;
}

enum NeTextureFormat
Re_SwapchainFormat(struct NeSwapchain *sw)
{
	return MTLToNeTextureFormat([sw->layer pixelFormat]);
}

void
Re_SwapchainDesc(struct NeSwapchain *sw, struct NeFramebufferAttachmentDesc *desc)
{
	desc->format = MTLToNeTextureFormat([sw->layer pixelFormat]);
	desc->usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST;
}

struct NeTexture *
Re_SwapchainTexture(struct NeSwapchain *sw, void *image)
{
	struct NeTexture *t = Sys_Alloc(1, sizeof(*t), MH_Transient);
	t->tex = [(id<CAMetalDrawable>)image texture];
	return t;
}

void
Re_ScreenResized(struct NeSwapchain *sw)
{
}
