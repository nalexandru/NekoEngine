#include <assert.h>

#include <System/Memory.h>
#include <Runtime/Runtime.h>

#include "NullGraphicsDriver.h"

struct NeRenderContext *
NG_CreateContext(struct NeRenderDevice *dev)
{
	struct NeRenderContext *ctx = Sys_Alloc(1, sizeof(*ctx), MH_RenderDriver);
	if (!ctx)
		return NULL;

	return ctx;
}

void NG_ResetContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx) { }
void NG_DestroyContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx) { }
static NeCommandBufferHandle _BeginSecondary(struct NeRenderContext *ctx, struct NeRenderPassDesc *passDesc) { return ctx; }
static void _BeginDrawCommandBuffer(struct NeRenderContext *ctx) { }
static void _BeginComputeCommandBuffer(struct NeRenderContext *ctx) { }
static void _BeginTransferCommandBuffer(struct NeRenderContext *ctx) { }
static NeCommandBufferHandle _EndCommandBuffer(struct NeRenderContext *ctx) { return ctx; }
static void _BindPipeline(struct NeRenderContext *ctx, struct NePipeline *pipeline) { }
static void _PushConstants(struct NeRenderContext *ctx, enum NeShaderStage stage, uint32_t size, const void *data) { }
static void _BindIndexBuffer(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, enum NeIndexType type) { }
static void _ExecuteSecondary(struct NeRenderContext *ctx, NeCommandBufferHandle *cmdBuffers, uint32_t count) { }
static void _BeginRenderPass(struct NeRenderContext *ctx, struct NeRenderPassDesc *passDesc, struct NeFramebuffer *fb, enum NeRenderCommandContents contents) { }
static void _EndRenderPass(struct NeRenderContext *ctx) { }
static void _SetViewport(struct NeRenderContext *ctx, float x, float y, float width, float height, float minDepth, float maxDepth) { }
static void _SetScissor(struct NeRenderContext *ctx, int32_t x, int32_t y, int32_t width, int32_t height) { }
static void _Draw(struct NeRenderContext *ctx, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) { }
static void _DrawIndexed(struct NeRenderContext *ctx, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) { }
static void _DrawIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride) { }
static void _DrawIndexedIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride) { }
static void _Dispatch(struct NeRenderContext *ctx, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) { }
static void _DispatchIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset) { }
static void _TraceRays(struct NeRenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth) { }
static void _TraceRaysIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset) { }
static void _BuildAccelerationStructures(struct NeRenderContext *ctx, uint32_t count, struct NeAccelerationStructureBuildInfo *buildInfo,
	const struct NeAccelerationStructureRangeInfo **rangeInfo) { }
static void _Barrier(struct NeRenderContext *ctx, enum NePipelineStage srcStage, enum NePipelineStage dstStage, enum NePipelineDependency dep,
	uint32_t memBarrierCount, const struct NeMemoryBarrier *memBarriers, uint32_t bufferBarrierCount, const struct NeBufferBarrier *bufferBarriers,
	uint32_t imageBarrierCount, const struct NeImageBarrier *imageBarriers) { }
static void _Transition(struct NeRenderContext *ctx, struct NeTexture *tex, enum NeTextureLayout newLayout) { }
static void _CopyBuffer(struct NeRenderContext *ctx, const struct NeBuffer *src, uint64_t srcOffset, struct NeBuffer *dst, uint64_t dstOffset, uint64_t size) { }
static void _CopyImage(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeTexture *dst) { }
static void _CopyBufferToTexture(struct NeRenderContext *ctx, const struct NeBuffer *src, struct NeTexture *dst, const struct NeBufferImageCopy *bic) { }
static void _CopyTextureToBuffer(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeBuffer *dst, const struct NeBufferImageCopy *bic) { }
static void _Blit(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeTexture *dst, const struct NeBlitRegion *regions, uint32_t regionCount, enum NeImageFilter filter) { }
static bool _QueueGraphics(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal) { return true; }
static bool _QueueCompute(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal) { return true; }
static bool _QueueTransfer(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal) { return true; }
static bool _ExecuteGraphics(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer) { return true; }
static bool _ExecuteCompute(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer) { return true; }
static bool _ExecuteTransfer(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer) { return true; }

void
NG_InitContextProcs(struct NeRenderContextProcs *p)
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
	p->QueueCompute = _QueueCompute;
	p->QueueGraphics = _QueueGraphics;
	p->QueueTransfer = _QueueTransfer;
	p->ExecuteCompute = _ExecuteCompute;
	p->ExecuteGraphics = _ExecuteGraphics;
	p->ExecuteTransfer = _ExecuteTransfer;
}
