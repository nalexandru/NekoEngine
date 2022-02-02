#include <assert.h>
#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#include "OpenGLDriver.h"

/*static inline bool _Create(VkDevice dev, VkPhysicalDevice physDev, struct NeSwapchain *sw);
static inline void _Submit(VkQueue queue, struct Array *submitInfo,
							uint32_t waitCount, VkSemaphore *wait, uint64_t *waitValues, VkPipelineStageFlags *waitStages,
							uint32_t signalCount, VkSemaphore *signal, const uint64_t *signalValues);*/

struct NeSurface *
GL_CreateSurface(struct NeRenderDevice *dev, void *window)
{
	struct NeSurface *s = Sys_Alloc(1, sizeof(*s), MH_RenderDriver);
	return s;
}

void
GL_DestroySurface(struct NeRenderDevice *dev, struct NeSurface *surface)
{
	Sys_Free(surface);
}

struct NeSwapchain *
GL_CreateSwapchain(struct NeRenderDevice *dev, struct NeSurface *surface, bool verticalSync)
{
	struct NeSwapchain *sw = Sys_Alloc(1, sizeof(*sw), MH_RenderDriver);
	return sw;
}

void
GL_DestroySwapchain(struct NeRenderDevice *dev, struct NeSwapchain *sw)
{
	Sys_Free(sw);
}

void *
GL_AcquireNextImage(struct NeRenderDevice *dev, struct NeSwapchain *sw)
{
	return sw;
}

bool
GL_Present(struct NeRenderDevice *dev, struct NeRenderContext *ctx, struct NeSwapchain *sw, void *image, struct NeSemaphore *waitSemaphore)
{
	GL_SwapBuffers();
	return true;
}

enum NeTextureFormat
GL_SwapchainFormat(struct NeSwapchain *sw)
{
	return TF_R8G8B8A8_UNORM;
}

void
GL_SwapchainDesc(struct NeSwapchain *sw, struct NeFramebufferAttachmentDesc *desc)
{
	desc->format = TF_R8G8B8A8_UNORM;
}

struct NeTexture *
GL_SwapchainTexture(struct NeSwapchain *sw, void *image)
{
	return image;
}

void
GL_ScreenResized(struct NeRenderDevice *dev, struct NeSwapchain *sw)
{
}
