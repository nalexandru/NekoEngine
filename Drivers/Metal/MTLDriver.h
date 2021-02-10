#ifndef _MTLDRIVER_H_
#define _MTLDRIVER_H_

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

struct RenderContext
{
	id<MTLCommandQueue> queue;
};


// Device functions
bool MTL_InitDevice(id<MTLDevice> dev);
void MTL_TermDevice(id<MTLDevice> dev);

struct RenderContext *MTL_CreateContext(id<MTLDevice> dev);
void MTL_DestroyContext(id<MTLDevice> dev, struct RenderContext *ctx);

void *MTL_CreateSurface(id<MTLDevice> dev, NSWindow *window);
void MTL_DestroySurface(id<MTLDevice> dev, NSView *view);

void *MTL_CreateSwapchain(id<MTLDevice> dev, NSView *view);
void MTL_DestroySwapchain(id<MTLDevice> dev, CAMetalLayer *layer);

void *MTL_AcquireNextImage(id<MTLDevice> dev, CAMetalLayer *layer);
bool MTL_Present(id<MTLDevice> dev, NSView *v, id<CAMetalDrawable> image);

// Pipeline
void MTL_LoadPipelineCache(id<MTLDevice> dev, struct Stream *stm);
void MTL_SavePipelineCache(id<MTLDevice> dev, struct Stream *stm);

#endif /* _MTLDRIVER_H_ */
