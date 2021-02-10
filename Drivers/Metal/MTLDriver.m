#define Handle __EngineHandle

#include <Render/Driver.h>
#include <Render/Device.h>
#include <Render/Context.h>

#undef Handle

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include "MTLDriver.h"

static bool _Init(void);
static void _Term(void);
static bool _EnumerateDevices(uint32_t *, struct RenderDeviceInfo *);
static struct RenderDevice *_CreateDevice(struct RenderDeviceInfo *info,
										  struct RenderDeviceProcs *devProcs,
										  struct RenderContextProcs *ctxProcs);

static struct RenderDriver _drv =
{
	NE_RENDER_DRIVER_ID,
	NE_RENDER_DRIVER_API,
	_Init,
	_Term,
	_EnumerateDevices,
	_CreateDevice
};

#ifdef RENDER_DRIVER_BUILTIN
const struct RenderDriver *Re_LoadBuiltinDriver() { return &_drv; }
#else
const struct RenderDriver *Re_LoadDriver() { return &_drv; }
#endif

static bool
_Init(void)
{
	return true;
}

static void
_Term(void)
{
}

static bool
_EnumerateDevices(uint32_t *count, struct RenderDeviceInfo *info)
{
	if (!count)
		return false;
	
	NSArray<id<MTLDevice>> *devices = MTLCopyAllDevices();
	
	if (!*count || !info) {
		*count = (uint32_t)[devices count];
		[devices release];
		return true;
	}
	
	for (uint32_t i = 0; i < *count; ++i) {
		id<MTLDevice> dev = devices[i];
		
		snprintf(info[i].deviceName, sizeof(info[i].deviceName), "%s", [[dev name] UTF8String]);
		info[i].localMemorySize = [dev recommendedMaxWorkingSetSize];
		
		info[i].features.meshShading = false;
		info[i].features.rayTracing = [dev supportsRaytracing];
		info[i].features.unifiedMemory = [dev hasUnifiedMemory];
		info[i].features.discrete = ![dev isLowPower];
		info[i].features.canPresent = ![dev isHeadless];
		
		info[i].limits.maxTextureSize = [dev supportsFamily: MTLGPUFamilyApple3]
						|| [dev supportsFamily: MTLGPUFamilyMac1] ? 16384 : 8192;
		
		info[i].private = (void *)devices[i];
	}
	
	[devices release];
	
	return true;
}

static struct RenderDevice *
_CreateDevice(struct RenderDeviceInfo *info,
			  struct RenderDeviceProcs *devProcs,
			  struct RenderContextProcs *ctxProcs)
{
	devProcs->Init = (bool(*)(struct RenderDevice *))MTL_InitDevice;
	devProcs->Term = (void(*)(struct RenderDevice *))MTL_TermDevice;
	
	devProcs->CreateContext = (struct RenderContext *(*)(struct RenderDevice *))MTL_CreateContext;
	devProcs->DestroyContext = (void(*)(struct RenderDevice *, struct RenderContext *))MTL_DestroyContext;
	
	devProcs->CreateSurface = (void *(*)(struct RenderDevice *, void *))MTL_CreateSurface;
	devProcs->DestroySurface = (void(*)(struct RenderDevice *, void *))MTL_DestroySurface;
	
	devProcs->CreateSwapchain = (void *(*)(struct RenderDevice *, void *))MTL_CreateSwapchain;
	devProcs->DestroySwapchain = (void(*)(struct RenderDevice *, void *))MTL_DestroySwapchain;
	
	devProcs->AcquireNextImage = (void *(*)(struct RenderDevice *, void *))MTL_AcquireNextImage;
	devProcs->Present = (bool(*)(struct RenderDevice *, void *, void *))MTL_Present;
	
	devProcs->LoadPipelineCache = (void(*)(struct RenderDevice *, struct Stream *))MTL_LoadPipelineCache;
	devProcs->SavePipelineCache = (void(*)(struct RenderDevice *, struct Stream *))MTL_LoadPipelineCache;
	
	return info->private;
}
