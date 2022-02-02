#include <stdlib.h>

#include "MTLDriver.h"

struct NeRenderContext *
MTL_CreateContext(id<MTLDevice> dev)
{
	struct NeRenderContext *ctx = Sys_Alloc(sizeof(*ctx), 1, MH_RenderDriver);
	
	ctx->queue = [dev newCommandQueue];
	ctx->dev = dev;

	if (!Rt_InitArray(&ctx->submitted.graphics, 10, sizeof(struct Mtld_SubmitInfo), MH_RenderDriver) ||
			!Rt_InitArray(&ctx->submitted.compute, 10, sizeof(struct Mtld_SubmitInfo), MH_RenderDriver) ||
			!Rt_InitArray(&ctx->submitted.xfer, 10, sizeof(struct Mtld_SubmitInfo), MH_RenderDriver))
		return nil;
	
	return ctx;
}

void
MTL_ResetContext(id<MTLDevice> dev, struct NeRenderContext *ctx)
{
	Rt_ClearArray(&ctx->submitted.graphics, false);
	Rt_ClearArray(&ctx->submitted.compute, false);
	Rt_ClearArray(&ctx->submitted.xfer, false);
}

void
MTL_DestroyContext(id<MTLDevice> dev, struct NeRenderContext *ctx)
{
	[ctx->queue release];

	Rt_TermArray(&ctx->submitted.graphics);
	Rt_TermArray(&ctx->submitted.compute);
	Rt_TermArray(&ctx->submitted.xfer);
	
	Sys_Free(ctx);
}

static NeCommandBufferHandle
_BeginSecondary(struct NeRenderContext *ctx, struct NeRenderPassDesc *passDesc)
{
	return ctx->cmdBuffer;
}

static void
_BeginDrawCommandBuffer(struct NeRenderContext *ctx)
{
	ctx->type = RC_RENDER;
	
	ctx->cmdBuffer = [ctx->queue commandBufferWithUnretainedReferences];
}

static void
_BeginComputeCommandBuffer(struct NeRenderContext *ctx)
{
	ctx->type = RC_COMPUTE;
	
	ctx->cmdBuffer = [ctx->queue commandBufferWithUnretainedReferences];
	ctx->encoders.compute = [ctx->cmdBuffer computeCommandEncoder];
}

static void
_BeginTransferCommandBuffer(struct NeRenderContext *ctx)
{
	ctx->type = RC_BLIT;
	
	ctx->cmdBuffer = [ctx->queue commandBufferWithUnretainedReferences];
	ctx->encoders.blit = [ctx->cmdBuffer blitCommandEncoder];
}

static NeCommandBufferHandle
_EndCommandBuffer(struct NeRenderContext *ctx)
{
	NeCommandBufferHandle cb = ctx->cmdBuffer;

	if (ctx->type == RC_COMPUTE)
		[ctx->encoders.compute endEncoding];
	else if (ctx->type == RC_BLIT)
		[ctx->encoders.blit endEncoding];

	ctx->boundPipeline = nil;

	return cb;
}

static void
_BindPipeline(struct NeRenderContext *ctx, struct NePipeline *pipeline)
{
	if (ctx->boundPipeline == pipeline)
		return;

	ctx->boundPipeline = pipeline;
	
	if (ctx->type == RC_RENDER) {
		[ctx->encoders.render setRenderPipelineState: pipeline->render.state];
		if (pipeline->render.depthStencil)
			[ctx->encoders.render setDepthStencilState: pipeline->render.depthStencil];

		MTLDrv_SetRenderHeaps(ctx->encoders.render);
		MTL_SetRenderArguments(ctx->encoders.render);
	} else if (ctx->type == RC_COMPUTE) {
		[ctx->encoders.compute setComputePipelineState: pipeline->compute.state];
		ctx->threadsPerThreadgroup = pipeline->compute.threadsPerThreadgroup;

		MTLDrv_SetComputeHeaps(ctx->encoders.compute);
		MTL_SetComputeArguments(ctx->encoders.compute);
	}
}

static void
_PushConstants(struct NeRenderContext *ctx, enum NeShaderStage stage, uint32_t size, const void *data)
{
	if (stage & SS_VERTEX || stage & SS_GEOMETRY || stage & SS_TESS_CTRL || stage & SS_TESS_EVAL)
		[ctx->encoders.render setVertexBytes: data length: size atIndex: 1];
	if (stage & SS_FRAGMENT)
		[ctx->encoders.render setFragmentBytes: data length: size atIndex: 1];
	if (ctx->type == RC_COMPUTE && stage & SS_COMPUTE)
		[ctx->encoders.compute setBytes: data length: size atIndex: 1];
}

static void
_BindIndexBuffer(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, enum NeIndexType type)
{
	ctx->boundIndexBuffer.buffer = buff;
	ctx->boundIndexBuffer.offset = offset;
	
	switch (type) {
	case IT_UINT_16: ctx->boundIndexBuffer.type = MTLIndexTypeUInt16; break;
	case IT_UINT_32: ctx->boundIndexBuffer.type = MTLIndexTypeUInt32; break;
	}
}

static void
_ExecuteSecondary(struct NeRenderContext *ctx, NeCommandBufferHandle *cmdBuffers, uint32_t count)
{
}

static void
_BeginRenderPass(struct NeRenderContext *ctx, struct NeRenderPassDesc *passDesc, struct NeFramebuffer *fb, enum NeRenderCommandContents contents)
{
	for (uint32_t i = 0; i < passDesc->colorAttachments; ++i)
		passDesc->desc.colorAttachments[i].texture = fb->attachments[i];
	
	if (passDesc->depthFormat != MTLPixelFormatInvalid && passDesc->colorAttachments < fb->attachmentCount)
		passDesc->desc.depthAttachment.texture = fb->attachments[passDesc->colorAttachments];

	for (uint32_t i = 0; i < passDesc->inputAttachments; ++i)
		passDesc->desc.colorAttachments[passDesc->colorAttachments + i].texture = fb->attachments[passDesc->colorAttachments + i + 1];

	ctx->encoders.render = [ctx->cmdBuffer renderCommandEncoderWithDescriptor: passDesc->desc];
}

static void
_EndRenderPass(struct NeRenderContext *ctx)
{
	[ctx->encoders.render endEncoding];
}

static void
_SetViewport(struct NeRenderContext *ctx, float x, float y, float width, float height, float minDepth, float maxDepth)
{
	MTLViewport vp = { .originX = x, .originY = y, .width = width, .height = height, .znear = minDepth, .zfar = maxDepth };
	[ctx->encoders.render setViewport: vp];
}

static void
_SetScissor(struct NeRenderContext *ctx, int32_t x, int32_t y, int32_t width, int32_t height)
{
	MTLScissorRect rc = { .x = x, .y = y, .width = width, .height = height };
	[ctx->encoders.render setScissorRect: rc];
}

static void
_Draw(struct NeRenderContext *ctx, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	[ctx->encoders.render drawPrimitives: ctx->boundPipeline->render.primitiveType
							 vertexStart: firstVertex
							 vertexCount: vertexCount
						   instanceCount: instanceCount
							baseInstance: firstInstance];
}

static void
_DrawIndexed(struct NeRenderContext *ctx, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
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

static void
_DrawIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	[ctx->encoders.render drawPrimitives: ctx->boundPipeline->render.primitiveType
						  indirectBuffer: buff->buff
					indirectBufferOffset: offset];
}

static void
_DrawIndexedIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	[ctx->encoders.render drawIndexedPrimitives: ctx->boundPipeline->render.primitiveType
									  indexType: ctx->boundIndexBuffer.type
									indexBuffer: ctx->boundIndexBuffer.buffer->buff
							  indexBufferOffset: ctx->boundIndexBuffer.offset
								 indirectBuffer: buff->buff
						   indirectBufferOffset: offset];
}

static void
_Dispatch(struct NeRenderContext *ctx, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	[ctx->encoders.compute dispatchThreadgroups: MTLSizeMake(groupCountX, groupCountY, groupCountZ)
						  threadsPerThreadgroup: ctx->threadsPerThreadgroup];
}

static void
_DispatchIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset)
{
	[ctx->encoders.compute dispatchThreadgroupsWithIndirectBuffer: buff->buff
											 indirectBufferOffset: offset
											threadsPerThreadgroup: ctx->threadsPerThreadgroup];
}

static void
_TraceRays(struct NeRenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth)
{
	_Dispatch(ctx, width, height, depth);
}

static void
_TraceRaysIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset)
{
	_DispatchIndirect(ctx, buff, offset);
}

static void
_BuildAccelerationStructures(struct NeRenderContext *ctx, uint32_t count, struct NeAccelerationStructureBuildInfo *buildInfo, const struct NeAccelerationStructureRangeInfo **rangeInfo)
{
/*	[ctx->encoders.accelerationStructure buildAccelerationStructure: as->as
														 descriptor: as->desc
													  scratchBuffer: scratch->buff
												scratchBufferOffset: 0];*/
}

static void
_Barrier(struct NeRenderContext *ctx, enum NePipelineStage srcStage, enum NePipelineStage dstStage, enum NePipelineDependency dep,
		 uint32_t memBarrierCount, const struct NeMemoryBarrier *memBarriers, uint32_t bufferBarrierCount, const struct NeBufferBarrier *bufferBarriers,
		 uint32_t imageBarrierCount, const struct NeImageBarrier *imageBarriers)
{
//	[ctx->encoders.render memoryBarrierWithResources:<#(id<MTLResource>  _Nonnull const * _Nonnull)#> count:<#(NSUInteger)#> afterStages:<#(MTLRenderStages)#> beforeStages:<#(MTLRenderStages)#>]
}

static void
_Transition(struct NeRenderContext *ctx, struct NeTexture *tex, enum NeTextureLayout newLayout)
{
	tex->layout = newLayout;
}

static void
_CopyBuffer(struct NeRenderContext *ctx, const struct NeBuffer *src, uint64_t srcOffset, struct NeBuffer *dst, uint64_t dstOffset, uint64_t size)
{
	[ctx->encoders.blit copyFromBuffer: src->buff
						  sourceOffset: srcOffset
							  toBuffer: dst->buff
					 destinationOffset: dstOffset
								  size: size];
}

static void
_CopyImage(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeTexture *dst)
{
	[ctx->encoders.blit copyFromTexture: src->tex toTexture: dst->tex];
}

static void
_CopyBufferToTexture(struct NeRenderContext *ctx, const struct NeBuffer *src, struct NeTexture *dst, const struct NeBufferImageCopy *bic)
{
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

static void
_CopyTextureToBuffer(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeBuffer *dst, const struct NeBufferImageCopy *bic)
{
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

static void
_Blit(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeTexture *dst, const struct NeBlitRegion *regions, uint32_t regionCount, enum NeImageFilter filter)
{
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

static bool
_QueueGraphics(struct NeRenderContext *ctx, id<MTLCommandBuffer> cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
	struct Mtld_SubmitInfo si =
	{
		.wait = wait ? wait->event : nil,
		.waitValue = wait ? wait->value : 0,
		.signal = signal ? signal->event : nil,
		.signalValue = signal ? ++signal->value : 0,
		.cmdBuffer = cmdBuffer
	};
	return Rt_ArrayAdd(&ctx->submitted.graphics, &si);
}

static bool
_QueueCompute(struct NeRenderContext *ctx, id<MTLCommandBuffer> cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
	struct Mtld_SubmitInfo si =
	{
		.wait = wait ? wait->event : nil,
		.waitValue = wait ? wait->value : 0,
		.signal = signal ? signal->event : nil,
		.signalValue = signal ? ++signal->value : 0,
		.cmdBuffer = cmdBuffer
	};
	return Rt_ArrayAdd(&ctx->submitted.compute, &si);
}

static bool
_QueueTransfer(struct NeRenderContext *ctx, id<MTLCommandBuffer> cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
	struct Mtld_SubmitInfo si =
	{
		.wait = wait ? wait->event : nil,
		.waitValue = wait ? wait->value : 0,
		.signal = signal ? signal->event : nil,
		.signalValue = signal ? ++signal->value : 0,
		.cmdBuffer = cmdBuffer
	};
	return Rt_ArrayAdd(&ctx->submitted.xfer, &si);
}

static bool
_Execute(struct NeRenderContext *ctx, id<MTLCommandBuffer> cmdBuffer)
{
	assert(ctx->type == RC_BLIT);

	__block dispatch_semaphore_t bds = dispatch_semaphore_create(0);
	[cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull cmdBuff) {
		dispatch_semaphore_signal(bds);
	}];
	[cmdBuffer commit];

	return dispatch_semaphore_wait(bds, UINT64_MAX) == 0;
}

void
MTL_InitContextProcs(struct NeRenderContextProcs *p)
{
	p->BeginSecondary = _BeginSecondary;
	p->BeginDrawCommandBuffer = _BeginDrawCommandBuffer;
	p->BeginComputeCommandBuffer = _BeginComputeCommandBuffer;
	p->BeginTransferCommandBuffer = _BeginTransferCommandBuffer;
	p->EndCommandBuffer = _EndCommandBuffer;
	p->BindPipeline = _BindPipeline;
	p->PushConstants = _PushConstants;
	p->BindIndexBuffer = _BindIndexBuffer;
	p->ExecuteSecondary = _ExecuteSecondary;
	p->BeginRenderPass = _BeginRenderPass;
	p->EndRenderPass = _EndRenderPass;
	p->SetViewport = _SetViewport;
	p->SetScissor = _SetScissor;
	p->Draw = _Draw;
	p->DrawIndexed = _DrawIndexed;
	p->DrawIndirect = _DrawIndirect;
	p->DrawIndexedIndirect = _DrawIndexedIndirect;
	p->Dispatch = _Dispatch;
	p->DispatchIndirect = _DispatchIndirect;
	p->TraceRays = _TraceRays;
	p->TraceRaysIndirect = _TraceRaysIndirect;
	p->BuildAccelerationStructures = _BuildAccelerationStructures;
	p->Barrier = _Barrier;
	p->Transition = _Transition;
	p->CopyBuffer = _CopyBuffer;
	p->CopyImage = _CopyImage;
	p->CopyBufferToTexture = _CopyBufferToTexture;
	p->CopyTextureToBuffer = _CopyTextureToBuffer;
	p->Blit = _Blit;
	p->QueueCompute = (bool(*)(struct NeRenderContext *, NeCommandBufferHandle, struct NeSemaphore *, struct NeSemaphore *))_QueueCompute;
	p->QueueGraphics = (bool(*)(struct NeRenderContext *, NeCommandBufferHandle, struct NeSemaphore *, struct NeSemaphore *))_QueueGraphics;
	p->QueueTransfer = (bool(*)(struct NeRenderContext *, NeCommandBufferHandle, struct NeSemaphore *, struct NeSemaphore *))_QueueTransfer;
	p->ExecuteCompute = (bool(*)(struct NeRenderContext *, NeCommandBufferHandle))_Execute;
	p->ExecuteGraphics = (bool(*)(struct NeRenderContext *, NeCommandBufferHandle))_Execute;
	p->ExecuteTransfer = (bool(*)(struct NeRenderContext *, NeCommandBufferHandle))_Execute;
}
