#ifndef _NE_RENDER_DRIVER_CONTEXT_H_
#define _NE_RENDER_DRIVER_CONTEXT_H_

#include <Engine/Job.h>
#include <Render/Types.h>
#include <System/Thread.h>

struct ImageSubresource
{
	enum ImageAspect aspect;
	uint32_t mipLevel;
	uint32_t baseArrayLayer;
	uint32_t layerCount;
	uint32_t levelCount;
};

struct BlitRegion
{
	struct ImageSubresource srcSubresource;
	struct {
		int32_t x, y, z;
	} srcOffset;
	struct {
		int32_t width, height, depth;
	} srcSize;
	struct ImageSubresource dstSubresource;
	struct {
		int32_t x, y, z;
	} dstOffset;
	struct {
		int32_t width, height, depth;
	} dstSize;
};

struct BufferImageCopy
{
	uint64_t bufferOffset;
	uint32_t bytesPerRow;
	uint32_t rowLength;
	uint32_t imageHeight;
	struct ImageSubresource subresource;
	struct {
		int32_t x, y, z;
	} imageOffset;
	struct {
		int32_t width, height, depth;
	} imageSize;
};

struct RenderContextProcs
{
	void (*Barrier)(struct RenderContext *dev, enum PipelineStage srcStage, enum PipelineStage dstStage, enum PipelineDependency dep,
		uint32_t memBarrierCount, const struct MemoryBarrier *memBarriers, uint32_t bufferBarrierCount, const struct BufferBarrier *bufferBarriers,
		uint32_t imageBarrierCount, const struct ImageBarrier *imageBarriers);

	CommandBufferHandle (*BeginSecondary)(struct RenderContext *ctx, struct RenderPassDesc *passDesc);
	void (*BeginDrawCommandBuffer)(struct RenderContext *ctx);
	void (*BeginComputeCommandBuffer)(struct RenderContext *ctx);
	void (*BeginTransferCommandBuffer)(struct RenderContext *ctx);
	void (*EndCommandBuffer)(struct RenderContext *ctx);

	void (*BindPipeline)(struct RenderContext *ctx, struct Pipeline *pipeline);
	void (*PushConstants)(struct RenderContext *ctx, enum ShaderStage stage, uint32_t size, const void *data);

	void (*BindIndexBuffer)(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, enum IndexType type);

	void (*BuildAccelerationStructures)(struct RenderContext *ctx, uint32_t count, struct AccelerationStructureBuildInfo *buildInfo, const struct AccelerationStructureRangeInfo **rangeInfo);

	void (*ExecuteSecondary)(struct RenderContext *ctx, CommandBufferHandle *cmdBuffers, uint32_t count);

	void (*BeginRenderPass)(struct RenderContext *ctx, struct RenderPassDesc *passDesc, struct Framebuffer *fb, enum RenderCommandContents contents);
	void (*EndRenderPass)(struct RenderContext *ctx);

	void (*SetViewport)(struct RenderContext *ctx, float x, float y, float width, float height, float minDepth, float maxDepth);
	void (*SetScissor)(struct RenderContext *ctx, int32_t x, int32_t y, int32_t width, int32_t height);

	void (*Draw)(struct RenderContext *ctx, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
	void (*DrawIndexed)(struct RenderContext *ctx, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
	void (*DrawIndirect)(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride);
	void (*DrawIndexedIndirect)(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride);

	void (*Dispatch)(struct RenderContext *ctx, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
	void (*DispatchIndirect)(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset);

	void (*TraceRays)(struct RenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth);
	void (*TraceRaysIndirect)(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset);

	void (*Transition)(struct RenderContext *ctx, struct Texture *tex, enum TextureLayout newLayout);

	void (*Present)(struct RenderContext *ctx);

	void (*CopyBuffer)(struct RenderContext *ctx, const struct Buffer *src, uint64_t srcOffset, struct Buffer *dst, uint64_t dstOffset, uint64_t size);
	void (*CopyImage)(struct RenderContext *ctx, const struct Texture *src, struct Texture *dst);
	void (*CopyBufferToTexture)(struct RenderContext *ctx, const struct Buffer *src, struct Texture *dst, const struct BufferImageCopy *bic);
	void (*CopyTextureToBuffer)(struct RenderContext *ctx, const struct Texture *src, struct Buffer *dst, const struct BufferImageCopy *bic);

	void (*Blit)(struct RenderContext *ctx, const struct Texture *src, struct Texture *dst, const struct BlitRegion *regions, uint32_t regionCount, enum ImageFilter filter);

	bool (*Submit)(struct RenderDevice *dev, struct RenderContext *ctx);
	bool (*SubmitTransfer)(struct RenderContext *ctx, struct Fence *f);
	bool (*SubmitCompute)(struct RenderContext *ctx);
};

ENGINE_API extern struct RenderDevice *Re_device;
ENGINE_API extern struct RenderContextProcs Re_contextProcs;
extern THREAD_LOCAL struct RenderContext *Re_context;
ENGINE_API extern struct RenderContext **Re_contexts;

static inline struct RenderContext *
Re_CurrentContext(void)
{
	if (!Re_context)
		Re_context = Re_contexts[E_WorkerId()];
	return Re_context;
}

void Re_CmdBindIndexBuffer(BufferHandle handle, uint64_t offset, enum IndexType type);

static inline CommandBufferHandle Re_BeginSecondary(struct RenderPassDesc *rpd) { return Re_contextProcs.BeginSecondary(Re_CurrentContext(), rpd); }
static inline void Re_BeginDrawCommandBuffer(void) { Re_contextProcs.BeginDrawCommandBuffer(Re_CurrentContext()); }
static inline void Re_BeginComputeCommandBuffer(void) { Re_contextProcs.BeginComputeCommandBuffer(Re_CurrentContext()); }
static inline void Re_BeginTransferCommandBuffer(void) { Re_contextProcs.BeginTransferCommandBuffer(Re_CurrentContext()); }
static inline void Re_EndCommandBuffer(void) { Re_contextProcs.EndCommandBuffer(Re_CurrentContext()); }

static inline void Re_CmdBindPipeline(struct Pipeline *pipeline) { Re_contextProcs.BindPipeline(Re_CurrentContext(), pipeline); }
static inline void Re_CmdPushConstants(enum ShaderStage stage, uint32_t size, const void *data)
{ Re_contextProcs.PushConstants(Re_CurrentContext(), stage, size, data); }

static inline void Re_CmdExecuteSecondary(CommandBufferHandle *cmdBuffers, uint32_t count) { Re_contextProcs.ExecuteSecondary(Re_CurrentContext(), cmdBuffers, count); }

static inline void Re_CmdBeginRenderPass(struct RenderPassDesc *passDesc, struct Framebuffer *fb, enum RenderCommandContents contents)
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
static inline void Re_CmdDrawIndirect(struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{ Re_contextProcs.DrawIndirect(Re_CurrentContext(), buff, offset, count, stride); }
static inline void Re_CmdDrawIndexedIndirect(struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{ Re_contextProcs.DrawIndexedIndirect(Re_CurrentContext(), buff, offset, count, stride); }

static inline void Re_CmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{ Re_contextProcs.Dispatch(Re_CurrentContext(), groupCountX, groupCountY, groupCountZ); }
static inline void Re_CmdDispatchIndirect(struct Buffer *buff, uint64_t offset)
{ Re_contextProcs.DispatchIndirect(Re_CurrentContext(), buff, offset); }

static inline void Re_CmdTraceRays(uint32_t width, uint32_t height, uint32_t depth) { Re_contextProcs.TraceRays(Re_CurrentContext(), width, height, depth); }
static inline void Re_CmdTraceRaysIndirect(struct Buffer *buff, uint64_t offset) { Re_contextProcs.TraceRaysIndirect(Re_CurrentContext(), buff, offset); }

static inline void Re_CmdBlit(const struct Texture *src, struct Texture *dst, const struct BlitRegion *regions, uint32_t regionCount, enum ImageFilter filter)
{ Re_contextProcs.Blit(Re_CurrentContext(), src, dst, regions, regionCount, filter); }

static inline bool Re_SubmitTransfer(struct Fence *f) { return Re_contextProcs.SubmitTransfer(Re_CurrentContext(), f); }
static inline bool Re_SubmitCompute(void) { return Re_contextProcs.SubmitCompute(Re_CurrentContext()); }

static inline void Re_Barrier(enum PipelineStage srcStage, enum PipelineStage dstStage, enum PipelineDependency dep,
	uint32_t memBarrierCount, struct MemoryBarrier *memBarriers, uint32_t bufferBarrierCount, struct BufferBarrier *bufferBarriers,
	uint32_t imageBarrierCount, struct ImageBarrier *imageBarriers)
{ Re_contextProcs.Barrier(Re_CurrentContext(), srcStage, dstStage, dep, memBarrierCount, memBarriers, bufferBarrierCount, bufferBarriers, imageBarrierCount, imageBarriers); }

#endif /* _NE_RENDER_DRIVER_CONTEXT_H_ */
