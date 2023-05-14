#include <System/Memory.h>
#include <Engine/Engine.h>

#include "MTLBackend.h"

static NSAutoreleasePool *_pool;
dispatch_semaphore_t MTL_frameSemaphore;

#if TARGET_OS_OSX

struct NeSurface *
Re_CreateSurface(void *window)
{
	((NSView *)window).wantsLayer = true;
	return (struct NeSurface *)window;
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
	struct NeSwapchain *sw = Sys_Alloc(1, sizeof(*sw), MH_RenderBackend);
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
	Rt_ArrayForEach(si, &ctx->submitted) {
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

void
Re_SwapchainTextureDesc(struct NeSwapchain *sw, struct NeTextureDesc *desc)
{
	desc->width = *E_screenWidth;
	desc->height = *E_screenHeight;
	desc->depth = 1;
	desc->format = MTLToNeTextureFormat([sw->layer pixelFormat]);
	desc->usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST;
	desc->type = TT_2D;
	desc->arrayLayers = 1;
	desc->mipLevels = 1;
	desc->gpuOptimalTiling = true;
	desc->memoryType = MT_GPU_LOCAL;
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

/* NekoEngine
 *
 * MTLSwapchain.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
