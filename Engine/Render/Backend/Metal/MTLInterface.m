#define RE_NATIVE_METAL

#include "MTLBackend.h"


static id<MTLCommandQueue> _IFace_CommandQueue(struct NeRenderContext *ctx) { return ctx->queue; }
static id<MTLCommandBuffer> _IFace_CurrentCommandBuffer(struct NeRenderContext *ctx) { return ctx->cmdBuffer; }
static id<MTLBlitCommandEncoder> _IFace_CurrentBlitEncoder(struct NeRenderContext *ctx) { return ctx->encoders.blit; }
static id<MTLRenderCommandEncoder> _IFace_CurrentRenderEncoder(struct NeRenderContext *ctx) { return ctx->encoders.render; }
static id<MTLComputeCommandEncoder> _IFace_CurrentComputeEncoder(struct NeRenderContext *ctx) { return ctx->encoders.compute; }

static id<MTLTexture> _IFace_Texture(struct NeTexture *tex) { return tex->tex; }
static id<MTLBuffer> _IFace_Buffer(struct NeBuffer *buff) { return buff->buff; }
static id<MTLAccelerationStructure> _IFace_AccelerationStructure(struct NeAccelerationStructure *as) { return as->as; }
static const MTLRenderPassDescriptor *_IFace_RenderPassDescriptor(struct NeRenderPassDesc *rpd) { return rpd->desc; }

struct NeRenderInterface *
Re_CreateRenderInterface(void)
{
	struct NeRenderInterface *iface = Sys_Alloc(sizeof(*iface), 1, MH_RenderDriver);
	if (!iface)
		return NULL;

	iface->CommandQueue = _IFace_CommandQueue;
	iface->CurrentCommandBuffer = _IFace_CurrentCommandBuffer;
	iface->CurrentBlitEncoder = _IFace_CurrentBlitEncoder;
	iface->CurrentRenderEncoder = _IFace_CurrentRenderEncoder;
	iface->CurrentComputeEncoder = _IFace_CurrentComputeEncoder;

	iface->Texture = _IFace_Texture;
	iface->Buffer = _IFace_Buffer;
	iface->AccelerationStructure = _IFace_AccelerationStructure;
	iface->RenderPassDescriptor = _IFace_RenderPassDescriptor;

	iface->device = Re_device->dev;

	return iface;
}

void
Re_DestroyRenderInterface(struct NeRenderInterface *iface)
{
	Sys_Free(iface);
}
