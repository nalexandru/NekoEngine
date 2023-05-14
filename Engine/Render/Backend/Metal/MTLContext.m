#include <stdlib.h>

#include "MTLBackend.h"

struct NeRenderContext *
Re_CreateContext(void)
{
	struct NeRenderContext *ctx = Sys_Alloc(sizeof(*ctx), 1, MH_RenderBackend);
	
	ctx->queue = [MTL_device newCommandQueue];

	if (!Rt_InitArray(&ctx->submitted, 10, sizeof(struct Mtld_SubmitInfo), MH_RenderBackend))
		return nil;

	if (!Rt_InitPtrArray(&ctx->resourceBarriers, 10, MH_RenderBackend))
		return nil;
	
	if (@available(macOS 13, iOS 16, *)) {
		NSError *err;
		MTLIOCommandQueueDescriptor *ioDesc = [[MTLIOCommandQueueDescriptor alloc] init];

		ioDesc.type = MTLIOCommandQueueTypeConcurrent;
		ioDesc.priority = MTLIOPriorityNormal;

		ctx->ioQueue = [MTL_device newIOCommandQueueWithDescriptor: ioDesc error: &err];
		ctx->ioCmdBuffer = [ctx->ioQueue commandBufferWithUnretainedReferences];

		[ioDesc release];
	}

	return ctx;
}

void
Re_ResetContext(struct NeRenderContext *ctx)
{
	Rt_ClearArray(&ctx->submitted, false);
}

void
Re_DestroyContext(struct NeRenderContext *ctx)
{
	if (@available(macOS 13, iOS 16, *))
		[ctx->ioQueue release];

	[ctx->queue release];

	Rt_TermArray(&ctx->resourceBarriers);
	Rt_TermArray(&ctx->submitted);
	
	Sys_Free(ctx);
}

NeCommandBufferHandle
Re_BeginSecondary(struct NeRenderPassDesc *passDesc)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	return ctx->cmdBuffer;
}

void
Re_BeginDrawCommandBuffer(struct NeSemaphore *wait)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	ctx->type = RC_RENDER;
	ctx->cmdBuffer = [ctx->queue commandBufferWithUnretainedReferences];
	
	if (wait)
		[ctx->cmdBuffer encodeWaitForEvent: wait->event value: wait->value];
}

void
Re_BeginComputeCommandBuffer(struct NeSemaphore *wait)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	ctx->type = RC_COMPUTE;
	ctx->cmdBuffer = [ctx->queue commandBufferWithUnretainedReferences];
	
	if (wait)
		[ctx->cmdBuffer encodeWaitForEvent: wait->event value: wait->value];
	
	ctx->encoders.compute = [ctx->cmdBuffer computeCommandEncoder];
}

void
Re_BeginTransferCommandBuffer(struct NeSemaphore *wait)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	ctx->type = RC_BLIT;
	ctx->cmdBuffer = [ctx->queue commandBuffer];
	
	if (wait)
		[ctx->cmdBuffer encodeWaitForEvent: wait->event value: wait->value];
	
	ctx->encoders.blit = [ctx->cmdBuffer blitCommandEncoder];
}

NeCommandBufferHandle
Re_EndCommandBuffer(void)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	NeCommandBufferHandle cb = ctx->cmdBuffer;

	if (ctx->type == RC_COMPUTE)
		[ctx->encoders.compute endEncoding];
	else if (ctx->type == RC_BLIT)
		[ctx->encoders.blit endEncoding];

	ctx->encoders.render = nil;
	ctx->boundPipeline = nil;
	
	return cb;
}

void
Re_CmdBindPipeline(struct NePipeline *pipeline)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	if (ctx->boundPipeline == pipeline)
		return;

	ctx->boundPipeline = pipeline;
	
	if (ctx->type == RC_RENDER && pipeline->type == PS_COMPUTE) {
		ctx->type = RC_COMPUTE;
		ctx->encoders.compute = [ctx->cmdBuffer computeCommandEncoder];
	}

	if (ctx->type == RC_RENDER) {
		[ctx->encoders.render setRenderPipelineState: pipeline->render.state];
		if (pipeline->render.depthStencil)
			[ctx->encoders.render setDepthStencilState: pipeline->render.depthStencil];

		MTLBk_SetRenderHeaps(ctx->encoders.render);
		MTL_SetRenderArguments(ctx->encoders.render);
	} else if (ctx->type == RC_COMPUTE) {
		[ctx->encoders.compute setComputePipelineState: pipeline->compute.state];
		ctx->threadsPerThreadgroup = pipeline->compute.threadsPerThreadgroup;

		MTLBk_SetComputeHeaps(ctx->encoders.compute);
		MTL_SetComputeArguments(ctx->encoders.compute);
	}
}

void
Re_CmdPushConstants(NeShaderStageFlags stage, uint32_t size, const void *data)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	if (stage & SS_VERTEX || stage & SS_GEOMETRY || stage & SS_TESS_CTRL || stage & SS_TESS_EVAL)
		[ctx->encoders.render setVertexBytes: data length: size atIndex: 1];
	if (stage & SS_FRAGMENT)
		[ctx->encoders.render setFragmentBytes: data length: size atIndex: 1];
	if (ctx->type == RC_COMPUTE && stage & SS_COMPUTE)
		[ctx->encoders.compute setBytes: data length: size atIndex: 1];
}

void
Re_BkCmdBindVertexBuffer(struct NeBuffer *buff, uint64_t offset)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	[ctx->encoders.render setVertexBuffer: buff->buff offset: offset atIndex: 2];
}

void
Re_BkCmdBindVertexBuffers(uint32_t count, struct NeBuffer **buffers, uint64_t *offsets, uint32_t start)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	id<MTLBuffer> *mbuffers = Sys_Alloc(sizeof(*mbuffers), count, MH_Frame);
	NSUInteger *moffsets = Sys_Alloc(sizeof(*moffsets), count, MH_Frame);

	for (uint32_t i = 0; i < count; ++i) {
		mbuffers[i] = buffers[i]->buff;
		moffsets[i] = offsets[i];
	}

	[ctx->encoders.render setVertexBuffers: mbuffers offsets: moffsets withRange: NSMakeRange(2 + start, count)];
}

void
Re_BkCmdBindIndexBuffer(struct NeBuffer *buff, uint64_t offset, enum NeIndexType type)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	ctx->boundIndexBuffer.buffer = buff;
	ctx->boundIndexBuffer.offset = offset;
	
	switch (type) {
	case IT_UINT_16: ctx->boundIndexBuffer.type = MTLIndexTypeUInt16; break;
	case IT_UINT_32: ctx->boundIndexBuffer.type = MTLIndexTypeUInt32; break;
	}
}

void
Re_CmdExecuteSecondary(NeCommandBufferHandle *cmdBuffers, uint32_t count)
{
}

void
Re_CmdBeginRenderPass(struct NeRenderPassDesc *passDesc, struct NeFramebuffer *fb, enum NeRenderCommandContents contents)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	const bool depth = passDesc->depthFormat != MTLPixelFormatInvalid;

	for (uint32_t i = 0; i < passDesc->colorAttachments; ++i)
		passDesc->desc.colorAttachments[i].texture = fb->attachments[i];
	
	if (depth && passDesc->colorAttachments < fb->attachmentCount)
		passDesc->desc.depthAttachment.texture = fb->attachments[passDesc->colorAttachments];

	for (uint32_t i = 0; i < passDesc->inputAttachments; ++i)
		passDesc->desc.colorAttachments[passDesc->colorAttachments + i].texture = fb->attachments[passDesc->colorAttachments + i + (depth ? 1 : 0)];

	ctx->encoders.render = [ctx->cmdBuffer renderCommandEncoderWithDescriptor: passDesc->desc];
	
	if (ctx->scopeBarrier) {
#if TARGET_OS_OSX
		const MTLBarrierScope scope = MTLBarrierScopeBuffers | MTLBarrierScopeRenderTargets | MTLBarrierScopeTextures;
#else
		const MTLBarrierScope scope = MTLBarrierScopeBuffers | MTLBarrierScopeTextures;
#endif
		[ctx->encoders.render memoryBarrierWithScope: scope
										 afterStages: MTLRenderStageFragment
										beforeStages: MTLRenderStageVertex];
		ctx->scopeBarrier = false;
	}
	
	if (ctx->resourceBarriers.count) {
		[ctx->encoders.render memoryBarrierWithResources: (id<MTLResource> *)ctx->resourceBarriers.data
												   count: ctx->resourceBarriers.count
											 afterStages: MTLRenderStageFragment
											beforeStages: MTLRenderStageVertex];
		Rt_ClearArray(&ctx->resourceBarriers, false);
	}
}

void
Re_CmdEndRenderPass(void)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	[ctx->encoders.render endEncoding];
}

void
Re_CmdSetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	MTLViewport vp = { .originX = x, .originY = y, .width = width, .height = height, .znear = minDepth, .zfar = maxDepth };
	[ctx->encoders.render setViewport: vp];
}

void
Re_CmdSetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	MTLScissorRect rc = { .x = x, .y = y, .width = width, .height = height };
	[ctx->encoders.render setScissorRect: rc];
}

void
Re_CmdSetLineWidth(float width)
{
    (void)width;
}

void
Re_CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	[ctx->encoders.render drawPrimitives: ctx->boundPipeline->render.primitiveType
							 vertexStart: firstVertex
							 vertexCount: vertexCount
						   instanceCount: instanceCount
							baseInstance: firstInstance];
}

void
Re_CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	NSUInteger indexOffset = ctx->boundIndexBuffer.offset;
	
	switch (ctx->boundIndexBuffer.type) {
	case MTLIndexTypeUInt16: indexOffset += sizeof(uint16_t) * firstIndex; break;
	case MTLIndexTypeUInt32: indexOffset += sizeof(uint32_t) * firstIndex; break;
	}
	
	[ctx->encoders.render drawIndexedPrimitives: ctx->boundPipeline->render.primitiveType
									 indexCount: indexCount
									  indexType: ctx->boundIndexBuffer.type
									indexBuffer: ctx->boundIndexBuffer.buffer->buff
							  indexBufferOffset: indexOffset
								  instanceCount: instanceCount
									 baseVertex: vertexOffset
								   baseInstance: firstInstance];
}

void
Re_CmdDrawIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	[ctx->encoders.render drawPrimitives: ctx->boundPipeline->render.primitiveType
						  indirectBuffer: buff->buff
					indirectBufferOffset: offset];
}

void
Re_CmdDrawIndexedIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	[ctx->encoders.render drawIndexedPrimitives: ctx->boundPipeline->render.primitiveType
									  indexType: ctx->boundIndexBuffer.type
									indexBuffer: ctx->boundIndexBuffer.buffer->buff
							  indexBufferOffset: ctx->boundIndexBuffer.offset
								 indirectBuffer: buff->buff
						   indirectBufferOffset: offset];
}

void
Re_CmdDrawMeshTasks(uint32_t taskCount, uint32_t firstTask)
{
	if (@available(macOS 13, iOS 16, *)) {
		struct NeRenderContext *ctx = Re_CurrentContext();
		[ctx->encoders.render drawMeshThreadgroups: MTLSizeMake(0, 0, 0)
					   threadsPerObjectThreadgroup: MTLSizeMake(0, 0, 0)
						 threadsPerMeshThreadgroup: MTLSizeMake(0, 0, 0)];
	}
}

void
Re_CmdDrawMeshTasksIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	if (@available(macOS 13, iOS 16, *)) {
		struct NeRenderContext *ctx = Re_CurrentContext();
		[ctx->encoders.render drawMeshThreadgroupsWithIndirectBuffer: buff->buff
												indirectBufferOffset: offset
										 threadsPerObjectThreadgroup: MTLSizeMake(0, 0, 0)
										   threadsPerMeshThreadgroup: MTLSizeMake(0, 0, 0)];
	}
}

void
Re_CmdDrawMeshTasksIndirectCount(struct NeBuffer *buff, uint64_t offset, struct NeBuffer *countBuff, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
{
	if (@available(macOS 13, iOS 16, *)) {
		struct NeRenderContext *ctx = Re_CurrentContext();
		[ctx->encoders.render drawMeshThreadgroupsWithIndirectBuffer: buff->buff
												indirectBufferOffset: offset
										 threadsPerObjectThreadgroup: MTLSizeMake(0, 0, 0)
										   threadsPerMeshThreadgroup: MTLSizeMake(0, 0, 0)];
	}
}

void
Re_CmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	[ctx->encoders.compute dispatchThreadgroups: MTLSizeMake(groupCountX, groupCountY, groupCountZ)
						  threadsPerThreadgroup: ctx->threadsPerThreadgroup];
}

void
Re_CmdDispatchIndirect(struct NeBuffer *buff, uint64_t offset)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	[ctx->encoders.compute dispatchThreadgroupsWithIndirectBuffer: buff->buff
											 indirectBufferOffset: offset
											threadsPerThreadgroup: ctx->threadsPerThreadgroup];
}

void
Re_CmdTraceRays(uint32_t width, uint32_t height, uint32_t depth)
{
	Re_CmdDispatch(width, height, depth);
}

void
Re_CmdTraceRaysIndirect(struct NeBuffer *buff, uint64_t offset)
{
	Re_CmdDispatchIndirect(buff, offset);
}

void
Re_CmdBuildAccelerationStructures(uint32_t count, struct NeAccelerationStructureBuildInfo *buildInfo, const struct NeAccelerationStructureRangeInfo **rangeInfo)
{
/*	struct NeRenderContext *ctx = Re_CurrentContext();

	[ctx->encoders.accelerationStructure buildAccelerationStructure: as->as
														 descriptor: as->desc
													  scratchBuffer: scratch->buff
												scratchBufferOffset: 0];*/
}

void
Re_CmdBarrier(enum NePipelineDependency dep,
	uint32_t memBarrierCount, const struct NeMemoryBarrier *memBarriers,
	uint32_t bufferBarrierCount, const struct NeBufferBarrier *bufferBarriers,
	uint32_t imageBarrierCount, const struct NeImageBarrier *imageBarriers)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	if (ctx->type == RC_RENDER) {
		ctx->scopeBarrier = memBarrierCount > 0;
		
		for (uint32_t i = 0; i < bufferBarrierCount; ++i)
			Rt_ArrayAddPtr(&ctx->resourceBarriers, bufferBarriers[i].buffer->buff);
		
		for (uint32_t i = 0; i < imageBarrierCount; ++i)
			Rt_ArrayAddPtr(&ctx->resourceBarriers, imageBarriers[i].texture->tex);
	} else if (ctx->type == RC_COMPUTE) {
		if (memBarrierCount)
#if TARGET_OS_OSX
			[ctx->encoders.compute memoryBarrierWithScope: MTLBarrierScopeBuffers | MTLBarrierScopeRenderTargets | MTLBarrierScopeTextures];
#else
		[ctx->encoders.compute memoryBarrierWithScope: MTLBarrierScopeBuffers | MTLBarrierScopeTextures];
#endif
		
		id<MTLResource> *res = Sys_Alloc(sizeof(*res), bufferBarrierCount + imageBarrierCount, MH_Frame);
		
		for (uint32_t i = 0; i < bufferBarrierCount; ++i)
			res[i] = bufferBarriers[i].buffer->buff;
		
		for (uint32_t i = 0; i < imageBarrierCount; ++i)
			res[bufferBarrierCount + i] = imageBarriers[i].texture->tex;
		
		[ctx->encoders.compute memoryBarrierWithResources: res count: bufferBarrierCount + imageBarrierCount];
	}
}

void
Re_CmdTransition(struct NeTexture *tex, enum NeTextureLayout newLayout)
{
	tex->layout = newLayout;
}

void
Re_BkCmdUpdateBuffer(const struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	
	__block id<MTLBuffer> staging = [[MTL_device newBufferWithBytes: data length: size options: MTLResourceStorageModeShared] autorelease];
	//[ctx->cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull cmdBuff) {
	//	[staging release];
	//}];
	
	[ctx->encoders.blit copyFromBuffer: staging
						  sourceOffset: 0
							  toBuffer: buff->buff
					 destinationOffset: offset
								  size: size];
}

void
Re_BkCmdCopyBuffer(const struct NeBuffer *src, uint64_t srcOffset, struct NeBuffer *dst, uint64_t dstOffset, uint64_t size)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	[ctx->encoders.blit copyFromBuffer: src->buff
						  sourceOffset: srcOffset
							  toBuffer: dst->buff
					 destinationOffset: dstOffset
								  size: size];
}

void
Re_CmdCopyImage(const struct NeTexture *src, struct NeTexture *dst)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	[ctx->encoders.blit copyFromTexture: src->tex toTexture: dst->tex];
}

void
Re_BkCmdCopyBufferToTexture(const struct NeBuffer *src, struct NeTexture *dst, const struct NeBufferImageCopy *bic)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	[ctx->encoders.blit copyFromBuffer: src->buff
						  sourceOffset: bic->bufferOffset
					 sourceBytesPerRow: bic->bytesPerRow
				   sourceBytesPerImage: 0
							sourceSize: MTLSizeMake(bic->imageSize.width, bic->imageSize.height, bic->imageSize.depth)
							 toTexture: dst->tex
					  destinationSlice: bic->subresource.baseArrayLayer
					  destinationLevel: bic->subresource.mipLevel
					 destinationOrigin: MTLOriginMake(bic->imageOffset.x, bic->imageOffset.y, bic->imageOffset.z)];
}

void
Re_BkCmdCopyTextureToBuffer(const struct NeTexture *src, struct NeBuffer *dst, const struct NeBufferImageCopy *bic)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	[ctx->encoders.blit copyFromTexture: src->tex
							sourceSlice: bic->subresource.baseArrayLayer
							sourceLevel: bic->subresource.mipLevel
						   sourceOrigin: MTLOriginMake(bic->imageOffset.x, bic->imageOffset.y, bic->imageOffset.z)
							 sourceSize: MTLSizeMake(bic->imageSize.width, bic->imageSize.height, bic->imageSize.depth)
							   toBuffer: dst->buff
					  destinationOffset: bic->bufferOffset
				 destinationBytesPerRow: bic->bytesPerRow
			   destinationBytesPerImage: 0];
}

void
Re_CmdBlit(const struct NeTexture *src, struct NeTexture *dst, const struct NeBlitRegion *regions, uint32_t regionCount, enum NeImageFilter filter)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	for (uint32_t i = 0; i < regionCount; ++i) {
		const struct NeBlitRegion r = regions[i];
		[ctx->encoders.blit copyFromTexture: src->tex
								sourceSlice: r.srcSubresource.baseArrayLayer
								sourceLevel: r.srcSubresource.mipLevel
							   sourceOrigin: MTLOriginMake(r.srcOffset.x, r.srcOffset.y, r.srcOffset.z)
								 sourceSize: MTLSizeMake(r.srcSize.width, r.srcSize.height, r.srcSize.depth)
								  toTexture: dst->tex
						   destinationSlice: r.dstSubresource.baseArrayLayer
						   destinationLevel: r.dstSubresource.mipLevel
						  destinationOrigin: MTLOriginMake(r.dstOffset.x, r.dstOffset.y, r.dstOffset.z)];
	}
}

void
Re_CmdLoadBuffer(struct NeBuffer *buff, uint64_t offset, uint64_t size, void *handle, uint64_t sourceOffset)
{
	if (@available(macOS 13, iOS 16, *)) {
		struct NeRenderContext *ctx = Re_CurrentContext();
		[ctx->ioCmdBuffer loadBuffer: buff->buff
							  offset: offset
								size: size
						sourceHandle: (id<MTLIOFileHandle>)handle
				  sourceHandleOffset: sourceOffset];
	}
}

void
Re_CmdLoadTexture(struct NeTexture *tex, uint32_t slice, uint32_t level, uint32_t width, uint32_t height,
				  uint32_t depth, uint32_t bytesPerRow, struct NeImageOffset *origin, void *handle, uint64_t sourceOffset)
{
	if (@available(macOS 13, iOS 16, *)) {
		struct NeRenderContext *ctx = Re_CurrentContext();
		[ctx->ioCmdBuffer loadTexture: tex->tex
								slice: slice
								level: level
								 size: MTLSizeMake(width, height, depth)
					sourceBytesPerRow: bytesPerRow
				  sourceBytesPerImage: 0
					destinationOrigin: MTLOriginMake(origin->x, origin->y, origin->z)
						 sourceHandle: (id<MTLIOFileHandle>)handle
				   sourceHandleOffset: sourceOffset];
	}
}

void
Re_BeginDirectIO(void)
{
	if (@available(macOS 13, iOS 16, *)) {
		struct NeRenderContext *ctx = Re_CurrentContext();
		ctx->ioCmdBuffer = [ctx->ioQueue commandBufferWithUnretainedReferences];
	}
}

bool
Re_SubmitDirectIO(bool *completed)
{
	if (@available(macOS 13, iOS 16, *)) {
		struct NeRenderContext *ctx = Re_CurrentContext();

		if (completed) {
			[ctx->ioCmdBuffer addCompletedHandler:^(id<MTLIOCommandBuffer> _Nonnull cmdBuff) {
				if (cmdBuff.status == MTLIOStatusPending)
					return;

				*completed = cmdBuff.status == MTLIOStatusComplete;
			}];
		}

		[ctx->ioCmdBuffer commit];
		return true;
	} else {
		return false;
	}
}

bool
Re_ExecuteDirectIO(void)
{
	if (@available(macOS 13, iOS 16, *)) {
		struct NeRenderContext *ctx = Re_CurrentContext();

		[ctx->ioCmdBuffer commit];

		__block bool ioResult = false;
		__block dispatch_semaphore_t bds = dispatch_semaphore_create(0);
		[ctx->ioCmdBuffer addCompletedHandler:^(id<MTLIOCommandBuffer> _Nonnull cmdBuff) {
			if (cmdBuff.status == MTLIOStatusPending)
				return;

			ioResult = cmdBuff.status == MTLIOStatusComplete;
			dispatch_semaphore_signal(bds);
		}];
		[ctx->ioCmdBuffer commit];

		bool rc = dispatch_semaphore_wait(bds, UINT64_MAX) == 0;

		dispatch_release(bds);

		return rc && ioResult;
	} else {
		return false;
	}
}

static inline bool
_Queue(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	struct Mtld_SubmitInfo si =
	{
		.signal = signal ? signal->event : nil,
		.signalValue = signal ? ++signal->value : 0,
		.cmdBuffer = cmdBuffer
	};
	return Rt_ArrayAdd(&ctx->submitted, &si);
}

static inline bool
_Execute(id<MTLCommandBuffer> cmdBuffer)
{
	__block dispatch_semaphore_t bds = dispatch_semaphore_create(0);
	[cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull cmdBuff) {
		dispatch_semaphore_signal(bds);
	}];
	[cmdBuffer commit];

	bool rc = dispatch_semaphore_wait(bds, UINT64_MAX) == 0;

	dispatch_release(bds);

	return rc;
}

void
Re_ExecuteNoWait(NeCommandBufferHandle cmdBuffer)
{
	id<MTLCommandBuffer> cb = cmdBuffer;
	[cb commit];
}

bool Re_QueueGraphics(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal) { return _Queue(cmdBuffer, signal); }
bool Re_QueueCompute(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal) { return _Queue(cmdBuffer, signal); }
bool Re_QueueTransfer(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal) { return _Queue(cmdBuffer, signal); }
bool Re_ExecuteGraphics(NeCommandBufferHandle cmdBuffer) { return _Execute(cmdBuffer); }
bool Re_ExecuteCompute(NeCommandBufferHandle cmdBuffer) { return _Execute(cmdBuffer); }
bool Re_ExecuteTransfer(NeCommandBufferHandle cmdBuffer) { return _Execute(cmdBuffer); }

/* NekoEngine
 *
 * MTLContext.m
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
