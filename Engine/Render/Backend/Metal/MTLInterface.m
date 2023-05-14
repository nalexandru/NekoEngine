#define RE_NATIVE_METAL

#include "MTLBackend.h"


static id<MTLCommandQueue> IFace_CommandQueue(struct NeRenderContext *ctx) { return ctx->queue; }
static id<MTLCommandBuffer> IFace_CurrentCommandBuffer(struct NeRenderContext *ctx) { return ctx->cmdBuffer; }
static id<MTLBlitCommandEncoder> IFace_CurrentBlitEncoder(struct NeRenderContext *ctx) { return ctx->encoders.blit; }
static id<MTLRenderCommandEncoder> IFace_CurrentRenderEncoder(struct NeRenderContext *ctx) { return ctx->encoders.render; }
static id<MTLComputeCommandEncoder> IFace_CurrentComputeEncoder(struct NeRenderContext *ctx) { return ctx->encoders.compute; }

static id<MTLTexture> IFace_Texture(struct NeTexture *tex) { return tex->tex; }
static id<MTLBuffer> IFace_Buffer(struct NeBuffer *buff) { return buff->buff; }
static id<MTLAccelerationStructure> IFace_AccelerationStructure(struct NeAccelerationStructure *as) { return as->as; }
static const MTLRenderPassDescriptor *IFace_RenderPassDescriptor(struct NeRenderPassDesc *rpd) { return rpd->desc; }

struct NeRenderInterface *
Re_CreateRenderInterface(void)
{
	struct NeRenderInterface *iface = Sys_Alloc(sizeof(*iface), 1, MH_RenderBackend);
	if (!iface)
		return NULL;

	iface->CommandQueue = IFace_CommandQueue;
	iface->CurrentCommandBuffer = IFace_CurrentCommandBuffer;
	iface->CurrentBlitEncoder = IFace_CurrentBlitEncoder;
	iface->CurrentRenderEncoder = IFace_CurrentRenderEncoder;
	iface->CurrentComputeEncoder = IFace_CurrentComputeEncoder;

	iface->Texture = IFace_Texture;
	iface->Buffer = IFace_Buffer;
	iface->AccelerationStructure = IFace_AccelerationStructure;
	iface->RenderPassDescriptor = IFace_RenderPassDescriptor;

	iface->device = Re_device->dev;

	return iface;
}

void
Re_DestroyRenderInterface(struct NeRenderInterface *iface)
{
	Sys_Free(iface);
}

/* NekoEngine
 *
 * MTLInterface.m
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
