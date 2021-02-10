#define Handle __EngineHandle

#include <Render/Device.h>

#undef Handle

#include "MTLDriver.h"

/*bool
MTL_InitDevice(id<MTLDevice> dev)
{
	return true;
}

struct RenderContext *
MTL_CreateContext(id<MTLDevice> dev)
{
	struct RenderContext *ctx = malloc(sizeof(*ctx));
	
	ctx->queue = [dev newCommandQueue];
	
	return ctx;
}*/

void
MTL_TraceRays(struct RenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth)
{
//	id<MTLCommandBuffer> cmdBuffer = [ctx->queue commandBuffer];
}

void
MTL_TraceRaysIndirect(struct RenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth)
{
//	[ctx->queue release];
	
//	free(ctx);
}

/*void
MTL_TermDevice(id<MTLDevice> dev)
{
	[dev release];
}*/
