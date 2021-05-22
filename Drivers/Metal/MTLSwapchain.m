#define Handle __EngineHandle

#include <System/Memory.h>

#undef Handle

#include "MTLDriver.h"

static NSAutoreleasePool *pool;

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

#endif

void *
MTL_CreateSwapchain(id<MTLDevice> dev, VIEWTYPE *view)
{
	CAMetalLayer *layer = (CAMetalLayer *)[view layer];
	layer.device = dev;
	return [view layer];
}

void
MTL_DestroySwapchain(id<MTLDevice> dev, CAMetalLayer *layer)
{
	(void)dev;
	(void)layer;
}

void *
MTL_AcquireNextImage(id<MTLDevice> dev, CAMetalLayer *layer)
{
	pool = [[NSAutoreleasePool alloc] init];
	return [layer nextDrawable];
}

bool
MTL_Present(id<MTLDevice> dev, struct RenderContext *ctx, VIEWTYPE *v, id<CAMetalDrawable> image)
{
	[ctx->cmdBuffer presentDrawable: image];
	[ctx->cmdBuffer commit];

	[pool release];
	
	return true;
}

struct Texture *
MTL_SwapchainTexture(CAMetalLayer *layer, id<CAMetalDrawable> image)
{
	struct Texture *t = Sys_Alloc(1, sizeof(*t), MH_Transient);
	t->tex = [image texture];
	return t;
}

enum TextureFormat
MTL_SwapchainFormat(CAMetalLayer *layer)
{
	return MTLToNeTextureFormat([layer pixelFormat]);
}

void
MTL_ScreenResized(id<MTLDevice> dev, CAMetalLayer *layer)
{
	(void)dev;
	(void)layer;
}
