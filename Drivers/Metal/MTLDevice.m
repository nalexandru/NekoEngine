#define Handle __EngineHandle

#include <Render/Device.h>

#undef Handle

#import <Cocoa/Cocoa.h>

#include "MTLDriver.h"

bool
MTL_InitDevice(id<MTLDevice> dev)
{
	return true;
}

struct RenderContext *
MTL_CreateContext(id<MTLDevice> dev)
{
	struct RenderContext *ctx = malloc(sizeof(*ctx));
	
	ctx->queue = [dev newCommandQueue];
	
	return ctx;
}

void
MTL_DestroyContext(id<MTLDevice> dev, struct RenderContext *ctx)
{
	[ctx->queue release];
	
	free(ctx);
}

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

void *
MTL_CreateSwapchain(id<MTLDevice> dev, NSView *view)
{
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
	return [layer nextDrawable];
}

bool
MTL_Present(id<MTLDevice> dev, NSView *v, id<CAMetalDrawable> image)
{
	// present
	[image autorelease];
	
	return true;
}

void
MTL_TermDevice(id<MTLDevice> dev)
{
	[dev release];
}
