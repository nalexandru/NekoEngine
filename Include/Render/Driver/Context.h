#ifndef _NE_RENDER_DRIVER_CONTEXT_H_
#define _NE_RENDER_DRIVER_CONTEXT_H_

#include <Engine/Job.h>
#include <Render/Types.h>
#include <System/Thread.h>

struct NeImageSubresource
{
	enum NeImageAspect aspect;
	uint32_t mipLevel;
	uint32_t baseArrayLayer;
	uint32_t layerCount;
	uint32_t levelCount;
};

struct NeBlitRegion
{
	struct NeImageSubresource srcSubresource;
	struct {
		int32_t x, y, z;
	} srcOffset;
	struct {
		int32_t width, height, depth;
	} srcSize;
	struct NeImageSubresource dstSubresource;
	struct {
		int32_t x, y, z;
	} dstOffset;
	struct {
		int32_t width, height, depth;
	} dstSize;
};

struct NeBufferImageCopy
{
	uint64_t bufferOffset;
	uint32_t bytesPerRow;
	uint32_t rowLength;
	uint32_t imageHeight;
	struct NeImageSubresource subresource;
	struct {
		int32_t x, y, z;
	} imageOffset;
	struct {
		int32_t width, height, depth;
	} imageSize;
};

struct NeRenderContextProcs
{
	void (*Barrier)(struct NeRenderContext *dev, enum NePipelineStage srcStage, enum NePipelineStage dstStage, enum NePipelineDependency dep,
		uint32_t memBarrierCount, const struct NeMemoryBarrier *memBarriers, uint32_t bufferBarrierCount, const struct NeBufferBarrier *bufferBarriers,
		uint32_t imageBarrierCount, const struct NeImageBarrier *imageBarriers);

	NeCommandBufferHandle (*BeginSecondary)(struct NeRenderContext *ctx, struct NeRenderPassDesc *passDesc);
	void (*BeginDrawCommandBuffer)(struct NeRenderContext *ctx);
	void (*BeginComputeCommandBuffer)(struct NeRenderContext *ctx);
	void (*BeginTransferCommandBuffer)(struct NeRenderContext *ctx);
	NeCommandBufferHandle (*EndCommandBuffer)(struct NeRenderContext *ctx);

	void (*BindPipeline)(struct NeRenderContext *ctx, struct NePipeline *pipeline);
	void (*PushConstants)(struct NeRenderContext *ctx, enum NeShaderStage stage, uint32_t size, const void *data);

	void (*BindIndexBuffer)(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, enum NeIndexType type);

	void (*BuildAccelerationStructures)(struct NeRenderContext *ctx, uint32_t count, struct NeAccelerationStructureBuildInfo *buildInfo, const struct NeAccelerationStructureRangeInfo **rangeInfo);

	void (*ExecuteSecondary)(struct NeRenderContext *ctx, NeCommandBufferHandle *cmdBuffers, uint32_t count);

	void (*BeginRenderPass)(struct NeRenderContext *ctx, struct NeRenderPassDesc *passDesc, struct NeFramebuffer *fb, enum NeRenderCommandContents contents);
	void (*EndRenderPass)(struct NeRenderContext *ctx);

	void (*SetViewport)(struct NeRenderContext *ctx, float x, float y, float width, float height, float minDepth, float maxDepth);
	void (*SetScissor)(struct NeRenderContext *ctx, int32_t x, int32_t y, int32_t width, int32_t height);

	void (*Draw)(struct NeRenderContext *ctx, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
	void (*DrawIndexed)(struct NeRenderContext *ctx, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
	void (*DrawIndirect)(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride);
	void (*DrawIndexedIndirect)(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride);

	void (*Dispatch)(struct NeRenderContext *ctx, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
	void (*DispatchIndirect)(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset);

	void (*TraceRays)(struct NeRenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth);
	void (*TraceRaysIndirect)(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset);

	void (*Transition)(struct NeRenderContext *ctx, struct NeTexture *tex, enum NeTextureLayout newLayout);

	void (*Present)(struct NeRenderContext *ctx);

	void (*CopyBuffer)(struct NeRenderContext *ctx, const struct NeBuffer *src, uint64_t srcOffset, struct NeBuffer *dst, uint64_t dstOffset, uint64_t size);
	void (*CopyImage)(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeTexture *dst);
	void (*CopyBufferToTexture)(struct NeRenderContext *ctx, const struct NeBuffer *src, struct NeTexture *dst, const struct NeBufferImageCopy *bic);
	void (*CopyTextureToBuffer)(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeBuffer *dst, const struct NeBufferImageCopy *bic);

	void (*Blit)(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeTexture *dst, const struct NeBlitRegion *regions, uint32_t regionCount, enum NeImageFilter filter);

	bool (*QueueCompute)(struct NeRenderContext *ctx, NeCommandBufferHandle cb, struct NeSemaphore *wait, struct NeSemaphore *signal);
	bool (*QueueGraphics)(struct NeRenderContext *ctx, NeCommandBufferHandle cb, struct NeSemaphore *wait, struct NeSemaphore *signal);
	bool (*QueueTransfer)(struct NeRenderContext *ctx, NeCommandBufferHandle cb, struct NeSemaphore *wait, struct NeSemaphore *signal);

	bool (*ExecuteCompute)(struct NeRenderContext *ctx, NeCommandBufferHandle cb);
	bool (*ExecuteGraphics)(struct NeRenderContext *ctx, NeCommandBufferHandle cb);
	bool (*ExecuteTransfer)(struct NeRenderContext *ctx, NeCommandBufferHandle cb);
};

ENGINE_API extern struct NeRenderDevice *Re_device;
ENGINE_API extern struct NeRenderContextProcs Re_contextProcs;
extern THREAD_LOCAL struct NeRenderContext *Re_context;
ENGINE_API extern struct NeRenderContext **Re_contexts;

static inline struct NeRenderContext *
Re_CurrentContext(void)
{
	if (!Re_context)
		Re_context = Re_contexts[E_WorkerId()];
	return Re_context;
}

void Re_CmdBindIndexBuffer(NeBufferHandle handle, uint64_t offset, enum NeIndexType type);

static inline NeCommandBufferHandle Re_BeginSecondary(struct NeRenderPassDesc *rpd) { return Re_contextProcs.BeginSecondary(Re_CurrentContext(), rpd); }
static inline void Re_BeginDrawCommandBuffer(void) { Re_contextProcs.BeginDrawCommandBuffer(Re_CurrentContext()); }
static inline void Re_BeginComputeCommandBuffer(void) { Re_contextProcs.BeginComputeCommandBuffer(Re_CurrentContext()); }
static inline void Re_BeginTransferCommandBuffer(void) { Re_contextProcs.BeginTransferCommandBuffer(Re_CurrentContext()); }
static inline NeCommandBufferHandle Re_EndCommandBuffer(void) { return Re_contextProcs.EndCommandBuffer(Re_CurrentContext()); }

static inline void Re_CmdBindPipeline(struct NePipeline *pipeline) { Re_contextProcs.BindPipeline(Re_CurrentContext(), pipeline); }
static inline void Re_CmdPushConstants(enum NeShaderStage stage, uint32_t size, const void *data)
{ Re_contextProcs.PushConstants(Re_CurrentContext(), stage, size, data); }

static inline void Re_CmdExecuteSecondary(NeCommandBufferHandle *cmdBuffers, uint32_t count) { Re_contextProcs.ExecuteSecondary(Re_CurrentContext(), cmdBuffers, count); }

static inline void Re_CmdBeginRenderPass(struct NeRenderPassDesc *passDesc, struct NeFramebuffer *fb, enum NeRenderCommandContents contents)
{ Re_contextProcs.BeginRenderPass(Re_CurrentContext(), passDesc, fb, contents); }
static inline void Re_CmdEndRenderPass(void) { Re_contextProcs.EndRenderPass(Re_CurrentContext()); }

static inline void Re_CmdSetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{ Re_contextProcs.SetViewport(Re_CurrentContext(), x, y, width, height, minDepth, maxDepth); }
static inline void Re_CmdSetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{ Re_contextProcs.SetScissor(Re_CurrentContext(), x, y, width, height); }

static inline void Re_CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{ Re_contextProcs.Draw(Re_CurrentContext(), vertexCount, instanceCount, firstVertex, firstInstance); }
static inline void Re_CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{ Re_contextProcs.DrawIndexed(Re_CurrentContext(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance); }
static inline void Re_CmdDrawIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{ Re_contextProcs.DrawIndirect(Re_CurrentContext(), buff, offset, count, stride); }
static inline void Re_CmdDrawIndexedIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{ Re_contextProcs.DrawIndexedIndirect(Re_CurrentContext(), buff, offset, count, stride); }

static inline void Re_CmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{ Re_contextProcs.Dispatch(Re_CurrentContext(), groupCountX, groupCountY, groupCountZ); }
static inline void Re_CmdDispatchIndirect(struct NeBuffer *buff, uint64_t offset)
{ Re_contextProcs.DispatchIndirect(Re_CurrentContext(), buff, offset); }

static inline void Re_CmdTraceRays(uint32_t width, uint32_t height, uint32_t depth) { Re_contextProcs.TraceRays(Re_CurrentContext(), width, height, depth); }
static inline void Re_CmdTraceRaysIndirect(struct NeBuffer *buff, uint64_t offset) { Re_contextProcs.TraceRaysIndirect(Re_CurrentContext(), buff, offset); }

static inline void Re_CmdBlit(const struct NeTexture *src, struct NeTexture *dst, const struct NeBlitRegion *regions, uint32_t regionCount, enum NeImageFilter filter)
{ Re_contextProcs.Blit(Re_CurrentContext(), src, dst, regions, regionCount, filter); }

static inline bool Re_QueueCompute(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{ return Re_contextProcs.QueueCompute(Re_CurrentContext(), cmdBuffer, wait, signal); }
static inline bool Re_QueueGraphics(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{ return Re_contextProcs.QueueGraphics(Re_CurrentContext(), cmdBuffer, wait, signal); }
static inline bool Re_QueueTransfer(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{ return Re_contextProcs.QueueTransfer(Re_CurrentContext(), cmdBuffer, wait, signal); }

static inline bool Re_ExecuteCompute(NeCommandBufferHandle cmdBuffer) { return Re_contextProcs.ExecuteCompute(Re_CurrentContext(), cmdBuffer); }
static inline bool Re_ExecuteGraphics(NeCommandBufferHandle cmdBuffer) { return Re_contextProcs.ExecuteGraphics(Re_CurrentContext(), cmdBuffer); }
static inline bool Re_ExecuteTransfer(NeCommandBufferHandle cmdBuffer) { return Re_contextProcs.ExecuteTransfer(Re_CurrentContext(), cmdBuffer); }

static inline void Re_Barrier(enum NePipelineStage srcStage, enum NePipelineStage dstStage, enum NePipelineDependency dep,
	uint32_t memBarrierCount, struct NeMemoryBarrier *memBarriers, uint32_t bufferBarrierCount, struct NeBufferBarrier *bufferBarriers,
	uint32_t imageBarrierCount, struct NeImageBarrier *imageBarriers)
{ Re_contextProcs.Barrier(Re_CurrentContext(), srcStage, dstStage, dep, memBarrierCount, memBarriers, bufferBarrierCount, bufferBarriers, imageBarrierCount, imageBarriers); }

#endif /* _NE_RENDER_DRIVER_CONTEXT_H_ */
