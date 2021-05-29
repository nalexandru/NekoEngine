#include <stdlib.h>

#include "MTLDriver.h"

struct RenderContext *
MTL_CreateContext(id<MTLDevice> dev)
{
	struct RenderContext *ctx = Sys_Alloc(sizeof(*ctx), 1, MH_RenderDriver);
	
	ctx->queue = [dev newCommandQueue];
	
	return ctx;
}

void
MTL_ResetContext(id<MTLDevice> dev, struct RenderContext *ctx)
{
}

void
MTL_DestroyContext(id<MTLDevice> dev, struct RenderContext *ctx)
{
	[ctx->queue release];
	
	Sys_Free(ctx);
}

static CommandBufferHandle
_BeginSecondary(struct RenderContext *ctx, struct RenderPassDesc *passDesc)
{
	return ctx->cmdBuffer;
}

static void
_BeginDrawCommandBuffer(struct RenderContext *ctx)
{
	ctx->type = RC_RENDER;
	
	ctx->cmdBuffer = [ctx->queue commandBuffer];
}

static void
_BeginComputeCommandBuffer(struct RenderContext *ctx)
{
	ctx->type = RC_COMPUTE;
	
	ctx->cmdBuffer = [ctx->queue commandBuffer];
	ctx->encoders.compute = [ctx->cmdBuffer computeCommandEncoder];
}

static void
_BeginTransferCommandBuffer(struct RenderContext *ctx)
{
	ctx->type = RC_BLIT;
	
	ctx->cmdBuffer = [ctx->queue commandBuffer];
	ctx->encoders.blit = [ctx->cmdBuffer blitCommandEncoder];
}

static void
_EndCommandBuffer(struct RenderContext *ctx)
{
	if (ctx->type == RC_COMPUTE)
		[ctx->encoders.compute endEncoding];
	else if (ctx->type == RC_BLIT)
		[ctx->encoders.blit endEncoding];
	
	ctx->boundPipeline = nil;
}

static void
_BindPipeline(struct RenderContext *ctx, struct Pipeline *pipeline)
{
	ctx->boundPipeline = pipeline;
	
	if (ctx->type == RC_RENDER) {
		[ctx->encoders.render setRenderPipelineState: pipeline->render.state];
		MTL_SetRenderArguments(ctx->encoders.render);
		
		if (pipeline->render.depthStencil)
			[ctx->encoders.render setDepthStencilState: pipeline->render.depthStencil];
	} else if (ctx->type == RC_COMPUTE) {
		[ctx->encoders.compute setComputePipelineState: pipeline->compute.state];
		MTL_SetComputeArguments(ctx->encoders.compute);
		
		ctx->threadsPerThreadgroup = pipeline->compute.threadsPerThreadgroup;
	}
}

static void
_PushConstants(struct RenderContext *ctx, enum ShaderStage stage, uint32_t size, const void *data)
{
	if (stage & SS_VERTEX || stage & SS_GEOMETRY || stage & SS_TESS_CTRL || stage & SS_TESS_EVAL)
		[ctx->encoders.render setVertexBytes: data length: size atIndex: 1];
	if (stage & SS_FRAGMENT)
		[ctx->encoders.render setFragmentBytes: data length: size atIndex: 1];
	if (ctx->type == RC_COMPUTE && stage & SS_COMPUTE)
		[ctx->encoders.compute setBytes: data length: size atIndex: 1];
}

static void
_BindIndexBuffer(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, enum IndexType type)
{
	ctx->boundIndexBuffer.buffer = buff;
	ctx->boundIndexBuffer.offset = offset;
	
	switch (type) {
	case IT_UINT_16: ctx->boundIndexBuffer.type = MTLIndexTypeUInt16; break;
	case IT_UINT_32: ctx->boundIndexBuffer.type = MTLIndexTypeUInt32; break;
	}
}

static void
_ExecuteSecondary(struct RenderContext *ctx, CommandBufferHandle *cmdBuffers, uint32_t count)
{
}

static void
_BeginRenderPass(struct RenderContext *ctx, struct RenderPassDesc *passDesc, struct Framebuffer *fb, enum RenderCommandContents contents)
{
	for (uint32_t i = 0; i < passDesc->attachmentCount; ++i)
		passDesc->desc.colorAttachments[i].texture = fb->attachments[i];
	
	if (passDesc->depthFormat != MTLPixelFormatInvalid && passDesc->attachmentCount < fb->attachmentCount)
		passDesc->desc.depthAttachment.texture = fb->attachments[passDesc->attachmentCount];

	ctx->encoders.render = [ctx->cmdBuffer renderCommandEncoderWithDescriptor: passDesc->desc];
}

static void
_EndRenderPass(struct RenderContext *ctx)
{
	[ctx->encoders.render endEncoding];
}

static void
_SetViewport(struct RenderContext *ctx, float x, float y, float width, float height, float minDepth, float maxDepth)
{
	MTLViewport vp = { .originX = x, .originY = y, .width = width, .height = height, .znear = minDepth, .zfar = maxDepth };
	[ctx->encoders.render setViewport: vp];
}

static void
_SetScissor(struct RenderContext *ctx, int32_t x, int32_t y, int32_t width, int32_t height)
{
	MTLScissorRect rc = { .x = x, .y = y, .width = width, .height = height };
	[ctx->encoders.render setScissorRect: rc];
}

static void
_Draw(struct RenderContext *ctx, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	[ctx->encoders.render drawPrimitives: ctx->boundPipeline->render.primitiveType
							 vertexStart: firstVertex
							 vertexCount: vertexCount
						   instanceCount: instanceCount
							baseInstance: firstInstance];
}

static void
_DrawIndexed(struct RenderContext *ctx, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
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
_DrawIndirect(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	[ctx->encoders.render drawPrimitives: ctx->boundPipeline->render.primitiveType
						  indirectBuffer: buff->buff
					indirectBufferOffset: offset];
}

static void
_DrawIndexedIndirect(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	[ctx->encoders.render drawIndexedPrimitives: ctx->boundPipeline->render.primitiveType
									  indexType: ctx->boundIndexBuffer.type
									indexBuffer: ctx->boundIndexBuffer.buffer->buff
							  indexBufferOffset: ctx->boundIndexBuffer.offset
								 indirectBuffer: buff->buff
						   indirectBufferOffset: offset];
}

static void
_Dispatch(struct RenderContext *ctx, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	[ctx->encoders.compute dispatchThreadgroups: MTLSizeMake(groupCountX, groupCountY, groupCountZ)
						  threadsPerThreadgroup: ctx->threadsPerThreadgroup];
}

static void
_DispatchIndirect(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset)
{
	[ctx->encoders.compute dispatchThreadgroupsWithIndirectBuffer: buff->buff
											 indirectBufferOffset: offset
											threadsPerThreadgroup: ctx->threadsPerThreadgroup];
}

static void
_TraceRays(struct RenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth)
{
	_Dispatch(ctx, width, height, depth);
}

static void
_TraceRaysIndirect(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset)
{
	_DispatchIndirect(ctx, buff, offset);
}

static void
_BuildAccelerationStructures(struct RenderContext *ctx, uint32_t count, struct AccelerationStructureBuildInfo *buildInfo, const struct AccelerationStructureRangeInfo **rangeInfo)
{
/*	[ctx->encoders.accelerationStructure buildAccelerationStructure: as->as
														 descriptor: as->desc
													  scratchBuffer: scratch->buff
												scratchBufferOffset: 0];*/
}

static void
_Barrier(struct RenderContext *ctx)
{
//	[ctx->encoders.render memoryBarrierWithResources:<#(id<MTLResource>  _Nonnull const * _Nonnull)#> count:<#(NSUInteger)#> afterStages:<#(MTLRenderStages)#> beforeStages:<#(MTLRenderStages)#>]
}

static void
_Transition(struct RenderContext *ctx, struct Texture *tex, enum TextureLayout newLayout)
{
	tex->layout = newLayout;
}

static void
_CopyBuffer(struct RenderContext *ctx, const struct Buffer *src, uint64_t srcOffset, struct Buffer *dst, uint64_t dstOffset, uint64_t size)
{
	[ctx->encoders.blit copyFromBuffer: src->buff
						  sourceOffset: srcOffset
							  toBuffer: dst->buff
					 destinationOffset: dstOffset
								  size: size];
}

static void
_CopyImage(struct RenderContext *ctx, const struct Texture *src, struct Texture *dst)
{
	[ctx->encoders.blit copyFromTexture: src->tex toTexture: dst->tex];
}

static void
_CopyBufferToImage(struct RenderContext *ctx, const struct Buffer *src, struct Texture *dst, const struct BufferImageCopy *bic)
{
	[ctx->encoders.blit copyFromBuffer: src->buff
						  sourceOffset: bic->bufferOffset
					 sourceBytesPerRow: bic->rowLength
				   sourceBytesPerImage: 0
							sourceSize: MTLSizeMake(bic->imageSize.width, bic->imageSize.height, bic->imageSize.depth)
							 toTexture: dst->tex
					  destinationSlice: bic->subresource.baseArrayLayer
					  destinationLevel: bic->subresource.mipLevel
					 destinationOrigin: MTLOriginMake(bic->imageOffset.x, bic->imageOffset.y, bic->imageOffset.z)];
}

static void
_CopyImageToBuffer(struct RenderContext *ctx, const struct Texture *src, struct Buffer *dst, const struct BufferImageCopy *bic)
{
	[ctx->encoders.blit copyFromTexture: src->tex
							sourceSlice: bic->subresource.baseArrayLayer
							sourceLevel: bic->subresource.mipLevel
						   sourceOrigin: MTLOriginMake(bic->imageOffset.x, bic->imageOffset.y, bic->imageOffset.z)
							 sourceSize: MTLSizeMake(bic->imageSize.width, bic->imageSize.height, bic->imageSize.depth)
							   toBuffer: dst->buff
					  destinationOffset: bic->bufferOffset
				 destinationBytesPerRow: bic->rowLength
			   destinationBytesPerImage: 0];
}

static void
_Blit(struct RenderContext *ctx, const struct Texture *src, struct Texture *dst, const struct BlitRegion *regions, uint32_t regionCount, enum ImageFilter filter)
{
	for (uint32_t i = 0; i < regionCount; ++i) {
		const struct BlitRegion r = regions[i];
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
_Submit(id<MTLDevice> dev, struct RenderContext *ctx)
{
	return false;
}

void
MTL_InitContextProcs(struct RenderContextProcs *p)
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
	p->CopyBufferToImage = _CopyBufferToImage;
	p->CopyImageToBuffer = _CopyImageToBuffer;
	p->Blit = _Blit;
	p->Submit = (bool(*)(struct RenderDevice *, struct RenderContext *))_Submit;
}
