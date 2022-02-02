#include <assert.h>
#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#include "NullGraphicsDriver.h"

struct NeSurface *
NG_CreateSurface(struct NeRenderDevice *dev, void *window)
{
	struct NeSurface *s = Sys_Alloc(1, sizeof(*s), MH_RenderDriver);
	return s;
}

void
NG_DestroySurface(struct NeRenderDevice *dev, struct NeSurface *surface)
{
	Sys_Free(surface);
}

struct NeSwapchain *
NG_CreateSwapchain(struct NeRenderDevice *dev, struct NeSurface *surface, bool verticalSync)
{
	struct NeSwapchain *sw = Sys_Alloc(1, sizeof(*sw), MH_RenderDriver);
	return sw;
}

void
NG_DestroySwapchain(struct NeRenderDevice *dev, struct NeSwapchain *sw)
{
}

void *
NG_AcquireNextImage(struct NeRenderDevice *dev, struct NeSwapchain *sw)
{
	return sw;
}

bool
NG_Present(struct NeRenderDevice *dev, struct NeRenderContext *ctx, struct NeSwapchain *sw, void *image, struct NeSemaphore *waitSemaphore)
{
	return true;
}

enum NeTextureFormat
NG_SwapchainFormat(struct NeSwapchain *sw)
{
	return TF_INVALID;
}

void
NG_SwapchainDesc(struct NeSwapchain *sw, struct NeFramebufferAttachmentDesc *desc)
{
}

struct NeTexture *
NG_SwapchainTexture(struct NeSwapchain *sw, void *image)
{
	return image;
}

void
NG_ScreenResized(struct NeRenderDevice *dev, struct NeSwapchain *sw)
{
}
