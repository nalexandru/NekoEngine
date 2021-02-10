#ifndef _RE_CONTEXT_H_
#define _RE_CONTEXT_H_

#include <Engine/Types.h>

struct RenderContextProcs
{
	void (*BeginDrawCommandBuffer)(struct RenderContext *);
	void (*BeginComputeCommandBuffer)(struct RenderContext *);
	void (*BeginTransferCommandBuffer)(struct RenderContext *);
	void (*EndCommandBuffer)(struct RenderContext *);
	
	void (*BindPipeline)(struct RenderContext *);
	
	void (*ExecuteSecondary)(struct RenderContext *contexts, uint32_t count);
	
	void (*BeginRenderPass)(struct RenderContext *);
	void (*EndRenderPass)(struct RenderContext *);
	
	void (*Draw)(struct RenderContext *);
	void (*DrawIndexed)(struct RenderContext *);
	void (*DrawIndirect)(struct RenderContext *);
	void (*DrawIndexedIndirect)(struct RenderContext *);
	
	void (*Dispatch)(struct RenderContext *);
	void (*DispatchIndirect)(struct RenderContext *);
	
	void (*TraceRays)(struct RenderContext *, uint32_t, uint32_t, uint32_t);
	void (*TraceRaysIndirect)(struct RenderContext *, uint32_t, uint32_t, uint32_t);
	
	void (*BuildAccelerationStructure)(struct RenderContext *);
	void (*UpdateAccelerationStructure)(struct RenderContext *);
	
	void (*Barrier)(struct RenderContext *);
	void (*Transition)(struct RenderContext *);
	
	void (*CopyBuffer)(struct RenderContext *);
	void (*CopyImage)(struct RenderContext *);
	void (*CopyBufferToImage)(struct RenderContext *);
	void (*CopyImageToBuffer)(struct RenderContext *);
	
	void (*Blit)(struct RenderContext *);
	void (*Resolve)(struct RenderContext *);
	void (*Clear)(struct RenderContext *);
};

extern struct RenderContextProcs Re_ContextProcs;

static inline void Re_BindPipeline(struct RenderContext *ctx)
{
	Re_ContextProcs.BindPipeline(ctx);
}

static inline void Re_TraceRays(struct RenderContext *ctx,
	uint32_t width, uint32_t height, uint32_t depth)
{
	Re_ContextProcs.TraceRays(ctx, width, height, depth);
}

static inline void Re_TraceRaysIndirect(struct RenderContext *ctx,
	uint32_t width, uint32_t height, uint32_t depth)
{
	Re_ContextProcs.TraceRaysIndirect(ctx, width, height, depth);
}

#endif /* _RE_CONTEXT_H_ */
