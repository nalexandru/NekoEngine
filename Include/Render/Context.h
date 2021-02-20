#ifndef _RE_CONTEXT_H_
#define _RE_CONTEXT_H_

#include <Engine/Types.h>
#include <Render/Shader.h>

enum IndexType
{
	IT_UINT_16,
	IT_UINT_32
};

enum BlitFilter
{
	BF_NEAREST,
	BF_LINEAR,
	BF_CUBIC
};

enum ImageAspect
{
	IA_COLOR	= 0x00000001,
	IA_DEPTH	= 0x00000002,
	IA_STENCIL	= 0x00000004
};

struct ImageRegion
{
	void *a;
};

struct ImageSubresource
{
	enum ImageAspect aspect;
	uint32_t mipLevel;
	uint32_t baseArrayLayer;
	uint32_t layerCount;
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

struct RenderContextProcs
{
	void (*BeginDrawCommandBuffer)(struct RenderContext *ctx);
	void (*BeginComputeCommandBuffer)(struct RenderContext *ctx);
	void (*BeginTransferCommandBuffer)(struct RenderContext *ctx);
	void (*EndCommandBuffer)(struct RenderContext *ctx);
	
	void (*BindPipeline)(struct RenderContext *ctx, struct Pipeline *pipeline);
	
	void (*BindDescriptorSets)(struct RenderContext *ctx, struct PipelineLayout *layout, uint32_t firstSet, uint32_t count, struct DescriptorSet *sets);
	void (*PushConstants)(struct RenderContext *ctx, struct PipelineLayout *layout, enum ShaderStage stage, uint32_t size, const void *data);
	
	void (*BindVertexBuffers)(struct RenderContext *ctx, uint32_t count, struct Buffer **buffers, uint64_t *offsets);
	void (*BindIndexBuffer)(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, enum IndexType type);
	
	void (*ExecuteSecondary)(struct RenderContext *ctx, struct RenderContext **contexts, uint32_t count);
	
	void (*BeginRenderPass)(struct RenderContext *ctx, struct RenderPass *pass, struct Framebuffer *fb);
	void (*EndRenderPass)(struct RenderContext *ctx);
	
	void (*Draw)(struct RenderContext *ctx, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
	void (*DrawIndexed)(struct RenderContext *ctx, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
	void (*DrawIndirect)(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride);
	void (*DrawIndexedIndirect)(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride);
	
	void (*Dispatch)(struct RenderContext *ctx, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
	void (*DispatchIndirect)(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset);
	
	void (*TraceRays)(struct RenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth);
	void (*TraceRaysIndirect)(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset);
	
	void (*BuildAccelerationStructure)(struct RenderContext *ctx, struct AccelerationStructure *as, struct Buffer *scratch);
	
	void (*Barrier)(struct RenderContext *ctx);
	void (*Transition)(struct RenderContext *ctx, struct Texture *tex);
	
	void (*Present)(struct RenderContext *ctx);
	
	void (*CopyBuffer)(struct RenderContext *ctx, struct Buffer *dst, uint64_t dstOffset, struct Buffer *src, uint64_t srcOffset, uint64_t size);
	void (*CopyImage)(struct RenderContext *ctx, struct Texture *dst, struct Texture *src);
	void (*CopyBufferToImage)(struct RenderContext *ctx, struct Buffer *src, struct Texture *dst);
	void (*CopyImageToBuffer)(struct RenderContext *ctx, struct Texture *src, struct Buffer *dst);
	
	void (*Blit)(struct RenderContext *ctx, struct Texture *dst, struct Texture *src, const struct BlitRegion *regions, uint32_t regionCount, enum BlitFilter filter);

	void (*Submit)(struct RenderDevice *dev, struct RenderContext *ctx);
};

extern struct RenderDevice *Re_device;
extern struct RenderContext **Re_contexts;
extern struct RenderContextProcs Re_contextProcs;

static inline struct RenderContext *Re_CurrentContext(void)
{
	return Re_contexts[0];
}

static inline void Re_BeginDrawCommandBuffer(void) { Re_contextProcs.BeginDrawCommandBuffer(Re_CurrentContext()); }
static inline void Re_BeginComputeCommandBuffer(void) { Re_contextProcs.BeginComputeCommandBuffer(Re_CurrentContext()); }
static inline void Re_BeginTransferCommandBuffer(void) { Re_contextProcs.BeginTransferCommandBuffer(Re_CurrentContext()); }
static inline void Re_EndCommandBuffer(void) { Re_contextProcs.EndCommandBuffer(Re_CurrentContext()); }

static inline void Re_BindPipeline(struct Pipeline *pipeline) { Re_contextProcs.BindPipeline(Re_CurrentContext(), pipeline); }

static inline void Re_BindVertexBuffers(uint32_t count, struct Buffer **buffers, uint64_t *offsets) { Re_contextProcs.BindVertexBuffers(Re_CurrentContext(), count, buffers, offsets); }
static inline void Re_BindIndexBuffer(struct Buffer *buff, uint64_t offset, enum IndexType type) { Re_contextProcs.BindIndexBuffer(Re_CurrentContext(), buff, offset, type); }

static inline void Re_ExecuteSecondary(struct RenderContext **contexts, uint32_t count) { Re_contextProcs.ExecuteSecondary(Re_CurrentContext(), contexts, count); }

static inline void Re_BeginRenderPass(struct RenderPass *pass, struct Framebuffer *fb) { Re_contextProcs.BeginRenderPass(Re_CurrentContext(), pass, fb); }
static inline void Re_EndRenderPass(void) { Re_contextProcs.EndRenderPass(Re_CurrentContext()); }

static inline void Re_Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{ Re_contextProcs.Draw(Re_CurrentContext(), vertexCount, instanceCount, firstVertex, firstInstance); }
static inline void Re_DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{ Re_contextProcs.DrawIndexed(Re_CurrentContext(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance); }
static inline void Re_DrawIndirect(struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{ Re_contextProcs.DrawIndirect(Re_CurrentContext(), buff, offset, count, stride); }
static inline void Re_DrawIndexedIndirect(struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{ Re_contextProcs.DrawIndexedIndirect(Re_CurrentContext(), buff, offset, count, stride); }

static inline void Re_Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{ Re_contextProcs.Dispatch(Re_CurrentContext(), groupCountX, groupCountY, groupCountZ); }
static inline void Re_DispatchIndirect(struct Buffer *buff, uint64_t offset)
{ Re_contextProcs.DispatchIndirect(Re_CurrentContext(), buff, offset); }

static inline void Re_TraceRays(uint32_t width, uint32_t height, uint32_t depth) { Re_contextProcs.TraceRays(Re_CurrentContext(), width, height, depth); }
static inline void Re_TraceRaysIndirect(struct Buffer *buff, uint64_t offset) { Re_contextProcs.TraceRaysIndirect(Re_CurrentContext(), buff, offset); }

#endif /* _RE_CONTEXT_H_ */
