#include <assert.h>
#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#include "GLBackend.h"

struct NeSwapchain *
Re_CreateSwapchain(struct NeSurface *surface, bool verticalSync)
{
	GLBk_EnableVerticalSync(verticalSync);
	return (struct NeSwapchain *)1;
}

void Re_DestroySwapchain(struct NeSwapchain *sw) { }

void *Re_AcquireNextImage(struct NeSwapchain *sw) { return (void *)(uint64_t)1; }

bool
Re_Present(struct NeSwapchain *sw, void *image, struct NeSemaphore *waitSemaphore)
{
	GLBk_ExecuteCommands();
	GLBk_SwapBuffers();
	return true;
}

enum NeTextureFormat Re_SwapchainFormat(struct NeSwapchain *sw) { return TL_UNKNOWN; }

void
Re_SwapchainDesc(struct NeSwapchain *sw, struct NeFramebufferAttachmentDesc *desc)
{
	desc->format = 0;//; VkToNeTextureFormat(sw->surfaceFormat.format);
	desc->usage = 0;// sw->imageUsage;
}

void
Re_SwapchainTextureDesc(struct NeSwapchain *sw, struct NeTextureDesc *desc)
{
	desc->width = *E_screenWidth;
	desc->height = *E_screenHeight;
	desc->depth = 1;
	desc->format = 0;// VkToNeTextureFormat(sw->surfaceFormat.format);
	desc->usage = 0;//sw->imageUsage;
	desc->type = TT_2D;
	desc->arrayLayers = 1;
	desc->mipLevels = 1;
	desc->gpuOptimalTiling = true;
	desc->memoryType = MT_GPU_LOCAL;
}

struct NeTexture *
Re_SwapchainTexture(struct NeSwapchain *sw, void *image)
{
	struct NeTexture *t = Sys_Alloc(sizeof(*t), 1, MH_Transient);
	t->id = 0;

	return t;
}

void Re_ScreenResized(struct NeSwapchain *sw) { }

/* NekoEngine
 *
 * GLSwapchain.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
