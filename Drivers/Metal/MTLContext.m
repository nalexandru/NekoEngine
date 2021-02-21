#define Handle __EngineHandle

#include <stdlib.h>

#include <Render/Device.h>
#include <Render/Context.h>

#undef Handle

#include "MTLDriver.h"

struct RenderContext *
MTL_CreateContext(id<MTLDevice> dev)
{
	struct RenderContext *ctx = malloc(sizeof(*ctx));
	
	ctx->queue = [dev newCommandQueue];
	
	return ctx;
}

void
MTL_DestroyContext(id<MTLDevice> dev, struct RenderContext *ctx)
{
	[ctx->queue release];
	
	free(ctx);
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
}

static void
_BindDescriptorSets(struct RenderContext *ctx, struct PipelineLayout *layout, uint32_t firstSet, uint32_t count, struct DescriptorSet *sets)
{
	// skip through the pipeline layout until firstSet
	// populate buffers for count sets
	//for (uint32_t)
	
	if (ctx->type == RC_RENDER) {
	//	[ctx->encoders.render setVertexBufferOffset:<#(NSUInteger)#> atIndex:<#(NSUInteger)#>]
	}
	
	// FIXME: is this correct ?
	/*if (stage & SS_VERTEX || stage & SS_GEOMETRY || stage & SS_TESS_CTRL || stage && SS_TESS_EVAL) {
		
	} else if (stage & SS_FRAGMENT) {
		
	}*/
}

static void
_PushConstants(struct RenderContext *ctx, struct PipelineLayout *layout, enum ShaderStage stage, uint32_t size, const void *data)
{
	if (stage & SS_VERTEX || stage & SS_GEOMETRY || stage & SS_TESS_CTRL || stage && SS_TESS_EVAL)
		[ctx->encoders.render setVertexBytes: data length: size atIndex: layout->pushConstantIndex];
	else if (stage & SS_FRAGMENT)
		[ctx->encoders.render setFragmentBytes: data length: size atIndex: layout->pushConstantIndex];
	else if (stage & SS_COMPUTE)
		[ctx->encoders.compute setBytes: data length: size atIndex: layout->pushConstantIndex];
}

static void
_BindVertexBuffers(struct RenderContext *ctx, uint32_t count, struct Buffer **buffers, uint64_t *offsets)
{
	for (uint32_t i = 0; i < count; ++i)
		[ctx->encoders.render setVertexBuffer: buffers[i]->buff offset: offsets[i] atIndex: i];
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
_ExecuteSecondary(struct RenderContext *ctx, struct RenderContext **contexts, uint32_t count)
{
	//
}

static void
_BeginRenderPass(struct RenderContext *ctx, struct RenderPass *pass, struct Framebuffer *fb)
{
	for (uint32_t i = 0; i < fb->attachmentCount; ++i)
		pass->desc.colorAttachments[i].texture = fb->attachments[i];
	
	ctx->encoders.render = [ctx->cmdBuffer renderCommandEncoderWithDescriptor: pass->desc];
}

static void
_EndRenderPass(struct RenderContext *ctx)
{
	[ctx->encoders.render endEncoding];
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
	
}

static void
_DrawIndexedIndirect(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	//
}

static void
_Dispatch(struct RenderContext *ctx, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
//	[ctx->encoders.compute dispatchThreadgroups:<#(MTLSize)#> threadsPerThreadgroup:<#(MTLSize)#>]
}

static void
_DispatchIndirect(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset)
{
//	pctx->encoders.compute dispatchThreadgroupsWithIndirectBuffer:<#(nonnull id<MTLBuffer>)#> indirectBufferOffset:<#(NSUInteger)#> threadsPerThreadgroup:<#(MTLSize)#>];
}

static void
_TraceRays(struct RenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth)
{
//	[ctx->encoders.render re]
}

static void
_TraceRaysIndirect(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset)
{
//
}

static void
_BuildAccelerationStructure(struct RenderContext *ctx, struct AccelerationStructure *as, struct Buffer *scratch)
{
	
}

static void
_Barrier(struct RenderContext *ctx)
{
	
}

static void
_Transition(struct RenderContext *ctx, struct Texture *tex)
{
	
}

static void
_CopyBuffer(struct RenderContext *ctx, struct Buffer *dst, uint64_t dstOffset, struct Buffer *src, uint64_t srcOffset, uint64_t size)
{
	[ctx->encoders.blit copyFromBuffer: src->buff
						  sourceOffset: srcOffset
							  toBuffer: dst->buff
					 destinationOffset: dstOffset
								  size: size];
}

static void
_CopyImage(struct RenderContext *ctx, struct Texture *dst, struct Texture *src)
{
	
}

static void
_CopyBufferToImage(struct RenderContext *ctx, struct Buffer *src, struct Texture *dst)
{
	//
}

static void
_CopyImageToBuffer(struct RenderContext *ctx, struct Texture *src, struct Buffer *dst)
{
	//
}

static void
_Blit(struct RenderContext *ctx, struct Texture *dst, struct Texture *src, const struct BlitRegion *regions, uint32_t regionCount, enum BlitFilter filter)
{
	
}

static bool
_Submit(id<MTLDevice> dev, struct RenderContext *ctx)
{
	return false;
}

void
MTL_InitContextProcs(struct RenderContextProcs *p)
{
	p->BeginDrawCommandBuffer = _BeginDrawCommandBuffer;
	p->BeginComputeCommandBuffer = _BeginComputeCommandBuffer;
	p->BeginTransferCommandBuffer = _BeginTransferCommandBuffer;
	p->EndCommandBuffer = _EndCommandBuffer;
	p->BindPipeline = _BindPipeline;
	p->BindDescriptorSets = _BindDescriptorSets;
	p->PushConstants = _PushConstants;
	p->BindVertexBuffers = _BindVertexBuffers;
	p->BindIndexBuffer = _BindIndexBuffer;
	p->ExecuteSecondary = _ExecuteSecondary;
	p->BeginRenderPass = _BeginRenderPass;
	p->EndRenderPass = _EndRenderPass;
	p->Draw = _Draw;
	p->DrawIndexed = _DrawIndexed;
	p->DrawIndirect = _DrawIndirect;
	p->DrawIndexedIndirect = _DrawIndexedIndirect;
	p->Dispatch = _Dispatch;
	p->DispatchIndirect = _DispatchIndirect;
	p->TraceRays = _TraceRays;
	p->TraceRaysIndirect = _TraceRaysIndirect;
	p->BuildAccelerationStructure = _BuildAccelerationStructure;
	p->Barrier = _Barrier;
	p->Transition = _Transition;
	p->CopyBuffer = _CopyBuffer;
	p->CopyImage = _CopyImage;
	p->CopyBufferToImage = _CopyBufferToImage;
	p->CopyImageToBuffer = _CopyImageToBuffer;
	p->Blit = _Blit;
	p->Submit = (bool(*)(struct RenderDevice *, struct RenderContext *))_Submit;
}
