#ifndef _MTLDRIVER_H_
#define _MTLDRIVER_H_

#define Handle __EngineHandle

#include <System/Log.h>
#include <System/Memory.h>
#include <Render/Types.h>
#include <Render/Render.h>

#undef Handle

#import <TargetConditionals.h>

#if TARGET_OS_OSX
#	import <Cocoa/Cocoa.h>
#	define VIEWTYPE		NSView
#	define WINDOWTYPE	NSWindow
#else
#	import <UIKit/UIKit.h>
#	import <MetalKit/MetalKit.h>
#	define VIEWTYPE		UIView
#	define WINDOWTYPE	UIWindow
#endif

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#define MTLDRV_MOD	L"MetalDriver"

#define PS_RENDER		0
#define PS_COMPUTE		1
#define PS_RAY_TRACING	2
struct Pipeline
{
	uint32_t type;
	union {
		struct {
			MTLPrimitiveType primitiveType;
			id<MTLRenderPipelineState> state;
			id<MTLDepthStencilState> depthStencil;
		} render;
		struct {
			id<MTLComputePipelineState> state;
			MTLSize threadsPerThreadgroup;
		} compute;
	};
};

struct Texture
{
	id<MTLTexture> tex;
	struct TextureDesc desc;
	enum TextureLayout layout;
};

struct Buffer
{
	id<MTLBuffer> buff;
	enum GPUMemoryType memoryType;
	uint32_t location;
};

struct RenderDevice
{
	id<MTLDevice> dev;
};

#define RC_RENDER	0
#define RC_COMPUTE	1
#define RC_BLIT		2
struct RenderContext
{
	uint32_t type;
	id<MTLCommandQueue> queue;
	id<MTLCommandBuffer> cmdBuffer;
	union {
		id<MTLBlitCommandEncoder> blit;
		id<MTLRenderCommandEncoder> render;
		id<MTLComputeCommandEncoder> compute;
		id<MTLAccelerationStructureCommandEncoder> accelerationStructure;
	} encoders;
	union {
		struct {
			struct Pipeline *boundPipeline;
			struct {
				MTLIndexType type;
				struct Buffer *buffer;
				uint64_t offset;
			} boundIndexBuffer;
		};
		MTLSize threadsPerThreadgroup;
	};
	id<MTLParallelRenderCommandEncoder> parallelEncoder;
	id<MTLDevice> dev;
};

struct Framebuffer
{
	id<MTLTexture> *attachments;
	uint32_t attachmentCount;
};

struct RenderPassDesc
{
	MTLRenderPassDescriptor *desc;
	uint32_t attachmentCount;
	MTLPixelFormat* attachmentFormats;
	MTLPixelFormat depthFormat;
};

struct AccelerationStructure
{
	id<MTLAccelerationStructure> as;
	MTLAccelerationStructureDescriptor *desc;
};

// Device
struct RenderDevice *MTL_CreateDevice(struct RenderDeviceInfo *info,
									  struct RenderDeviceProcs *devProcs,
									  struct RenderContextProcs *ctxProcs);
bool MTL_Execute(id<MTLDevice> dev, struct RenderContext *ctx, bool wait);
void MTL_WaitIdle(id<MTLDevice> dev);
void MTL_DestroyDevice(id<MTLDevice> dev);

// Pipeline
struct Pipeline *MTL_GraphicsPipeline(id<MTLDevice> dev, const struct GraphicsPipelineDesc *gpDesc);
struct Pipeline *MTL_ComputePipeline(id<MTLDevice> dev, const struct ComputePipelineDesc *cpDesc);
struct Pipeline *MTL_RayTracingPipeline(id<MTLDevice> dev, struct ShaderBindingTable *sbt, uint32_t maxDepth);
void MTL_LoadPipelineCache(id<MTLDevice> dev);
void MTL_SavePipelineCache(id<MTLDevice> dev);
void MTL_DestroyPipeline(id<MTLDevice> dev, struct Pipeline *p);

// Surface
void *MTL_CreateSurface(id<MTLDevice> dev, WINDOWTYPE *window);
void MTL_DestroySurface(id<MTLDevice> dev, VIEWTYPE *view);

// Swapchain
void *MTL_CreateSwapchain(id<MTLDevice> dev, VIEWTYPE *view);
void MTL_DestroySwapchain(id<MTLDevice> dev, CAMetalLayer *layer);
void *MTL_AcquireNextImage(id<MTLDevice> dev, CAMetalLayer *layer);
bool MTL_Present(id<MTLDevice> dev, struct RenderContext *ctx, VIEWTYPE *v, id<CAMetalDrawable> image);
struct Texture *MTL_SwapchainTexture(CAMetalLayer *layer, id<CAMetalDrawable> image);
enum TextureFormat MTL_SwapchainFormat(CAMetalLayer *layer);
void MTL_ScreenResized(id<MTLDevice> dev, CAMetalLayer *layer);

// Context
void MTL_InitContextProcs(struct RenderContextProcs *p);
struct RenderContext *MTL_CreateContext(id<MTLDevice> dev);
void MTL_ResetContext(id<MTLDevice> dev, struct RenderContext *ctx);
void MTL_DestroyContext(id<MTLDevice> dev, struct RenderContext *ctx);

// Texture
MTLTextureDescriptor *MTL_TextureDescriptor(id<MTLDevice> dev, const struct TextureCreateInfo *tci);
struct Texture *MTL_CreateTexture(id<MTLDevice> dev, const struct TextureCreateInfo *tci, uint16_t location);
enum TextureLayout MTL_TextureLayout(const struct Texture *tex);
void MTL_UploadTexture(id<MTLDevice> dev, id<MTLTexture> tex, const void *data, NSUInteger size, uint32_t width, uint32_t height, uint32_t depth);
void MTL_DestroyTexture(id<MTLDevice> dev, struct Texture *tex);

// Buffer
struct Buffer *MTL_CreateBuffer(id<MTLDevice> dev, const struct BufferCreateInfo *bci, uint16_t location);
void MTL_UpdateBuffer(id<MTLDevice> dev, struct Buffer *buff, uint64_t offset, uint8_t *data, uint64_t size);
void *MTL_MapBuffer(id<MTLDevice> dev, struct Buffer *buff);
void MTL_UnmapBuffer(id<MTLDevice> dev, struct Buffer *buff);
uint64_t MTL_BufferAddress(id<MTLDevice> dev, const struct Buffer *buff, uint64_t offset);
void MTL_DestroyBuffer(id<MTLDevice> dev, struct Buffer *buff);

// Acceleration Structure
struct AccelerationStructure *MTL_CreateAccelerationStructure(id<MTLDevice> dev, const struct AccelerationStructureCreateInfo *asci);
uint64_t MTL_AccelerationStructureHandle(id<MTLDevice> dev, const struct AccelerationStructure *as);
void MTL_DestroyAccelerationStructure(id<MTLDevice> dev, struct AccelerationStructure *as);

// Framebuffer
struct Framebuffer *MTL_CreateFramebuffer(id<MTLDevice> dev, const struct FramebufferDesc *desc);
void MTL_SetAttachment(struct Framebuffer *fb, uint32_t pos, struct Texture *tex);
const struct FramebufferDesc *MTL_FramebufferDesc(const struct Framebuffer *fb);
void MTL_DestroyFramebuffer(id<MTLDevice> dev, struct Framebuffer *fb);

// Render Pass
struct RenderPassDesc *MTL_CreateRenderPassDesc(id<MTLDevice> dev, const struct AttachmentDesc *attachments, uint32_t count, const struct AttachmentDesc *depthAttachment);
void MTL_DestroyRenderPassDesc(id<MTLDevice> dev, struct RenderPassDesc *pass);

// Argument Buffer
bool MTL_InitArgumentBuffer(id<MTLDevice> dev);
void MTL_TermArgumentBuffer(id<MTLDevice> dev);
void MTL_SetSampler(uint16_t location, id<MTLSamplerState> sampler);
void MTL_SetTexture(uint16_t location, id<MTLTexture> tex);
void MTL_RemoveTexture(id<MTLTexture> tex);
void MTL_SetBuffer(uint16_t location, id<MTLBuffer> buff);
void MTL_RemoveBuffer(id<MTLBuffer> buff);
void MTL_SetRenderArguments(id<MTLRenderCommandEncoder> encoder);
void MTL_SetComputeArguments(id<MTLComputeCommandEncoder> encoder);
void MTL_SetIndirectArguments(id<MTLRenderCommandEncoder> encoder, id<MTLIndirectRenderCommand> cmd);

// Shader
bool MTL_InitLibrary(id<MTLDevice> dev);
id<MTLFunction> MTL_ShaderModule(id<MTLDevice> dev, const char *name);
void MTL_TermLibrary(void);

// Sampler
id<MTLSamplerState> MTL_CreateSampler(id<MTLDevice> dev, const struct SamplerDesc *sDesc);
void MTL_DestroySampler(id<MTLDevice> dev, id<MTLSamplerState> s);

// Transient Resources
struct Texture *MTL_CreateTransientTexture(id<MTLDevice> dev, const struct TextureCreateInfo *tci, uint16_t location, uint64_t offset);
struct Buffer *MTL_CreateTransientBuffer(id<MTLDevice> dev, const struct BufferCreateInfo *bci, uint16_t location, uint64_t offset);
bool MTL_InitTransientHeap(id<MTLDevice> dev, uint64_t size);
bool MTL_ResizeTransientHeap(id<MTLDevice> dev, uint64_t size);
void MTL_TermTransientHeap(id<MTLDevice> dev);

// Utility functions
static inline MTLResourceOptions
MTL_GPUMemoryTypetoResourceOptions(bool hasUnifiedMemory, enum GPUMemoryType type)
{
#ifdef _DEBUG
	MTLResourceOptions options = MTLResourceHazardTrackingModeTracked;
#else
	MTLResourceOptions options = MTLResourceHazardTrackingModeUntracked;
#endif
	
	switch (type) {
	case MT_GPU_LOCAL: options = MTLResourceCPUCacheModeWriteCombined | MTLResourceStorageModePrivate; break;
#if TARGET_OS_OSX
	case MT_CPU_READ:
		if (hasUnifiedMemory)
			options = MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared;
		else
			options = MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeManaged;
	break;
	case MT_CPU_WRITE:
		if (hasUnifiedMemory)
			options = MTLResourceCPUCacheModeWriteCombined | MTLResourceStorageModeShared;
		else
			options = MTLResourceCPUCacheModeWriteCombined | MTLResourceStorageModeManaged;
	break;
#else
	case MT_CPU_READ: options = MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared; break;
	case MT_CPU_WRITE: options = MTLResourceCPUCacheModeWriteCombined | MTLResourceStorageModeShared; break;
#endif
	case MT_CPU_COHERENT: options = MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared; break;
	};
	
	return options;
}

static inline MTLPixelFormat
NeToMTLTextureFormat(enum TextureFormat fmt)
{
	switch (fmt) {
	case TF_R8G8B8A8_UNORM: return MTLPixelFormatRGBA8Unorm;
	case TF_R8G8B8A8_SRGB: return MTLPixelFormatRGBA8Unorm_sRGB;
	case TF_B8G8R8A8_UNORM: return MTLPixelFormatBGRA8Unorm;
	case TF_B8G8R8A8_SRGB: return MTLPixelFormatBGRA8Unorm_sRGB;
	case TF_R16G16B16A16_SFLOAT: return MTLPixelFormatRGBA16Float;
	case TF_R32G32B32A32_SFLOAT: return MTLPixelFormatRGBA32Float;
	case TF_A2R10G10B10_UNORM: return MTLPixelFormatRGB10A2Unorm;
	case TF_R8G8_UNORM: return MTLPixelFormatRG8Unorm;
	case TF_R8_UNORM: return MTLPixelFormatR8Unorm;
	case TF_ETC2_R8G8B8_UNORM: return MTLPixelFormatETC2_RGB8;
	case TF_ETC2_R8G8B8_SRGB: return MTLPixelFormatETC2_RGB8_sRGB;
	case TF_ETC2_R8G8B8A1_UNORM: return MTLPixelFormatETC2_RGB8A1;
	case TF_ETC2_R8G8B8A1_SRGB: return MTLPixelFormatETC2_RGB8A1_sRGB;
	case TF_EAC_R11_UNORM: return MTLPixelFormatEAC_R11Unorm;
	case TF_EAC_R11_SNORM: return MTLPixelFormatEAC_R11Snorm;
	case TF_EAC_R11G11_UNORM: return MTLPixelFormatEAC_RG11Unorm;
	case TF_EAC_R11G11_SNORM: return MTLPixelFormatEAC_RG11Snorm;
	case TF_D32_SFLOAT: return MTLPixelFormatDepth32Float;
	case TF_INVALID: return MTLPixelFormatInvalid;
#if TARGET_OS_OSX
	case TF_BC5_UNORM: return MTLPixelFormatBC5_RGUnorm;
	case TF_BC5_SNORM: return MTLPixelFormatBC5_RGSnorm;
	case TF_BC6H_UF16: return MTLPixelFormatBC6H_RGBUfloat;
	case TF_BC6H_SF16: return MTLPixelFormatBC6H_RGBFloat;
	case TF_BC7_UNORM: return MTLPixelFormatBC7_RGBAUnorm;
	case TF_BC7_SRGB: return MTLPixelFormatBC7_RGBAUnorm_sRGB;
	case TF_D24_STENCIL8: return MTLPixelFormatDepth24Unorm_Stencil8;
#else
	default: return MTLPixelFormatInvalid;
#endif
	}
	
	return MTLPixelFormatInvalid;
}

static inline enum TextureFormat
MTLToNeTextureFormat(MTLPixelFormat fmt)
{
	switch (fmt) {
	case MTLPixelFormatRGBA8Unorm: return TF_R8G8B8A8_UNORM;
	case MTLPixelFormatRGBA8Unorm_sRGB: return TF_R8G8B8A8_SRGB;
	case MTLPixelFormatBGRA8Unorm: return TF_B8G8R8A8_UNORM;
	case MTLPixelFormatBGRA8Unorm_sRGB: return TF_B8G8R8A8_SRGB;
	case MTLPixelFormatRGBA16Float: return TF_R16G16B16A16_SFLOAT;
	case MTLPixelFormatRGBA32Float: return TF_R32G32B32A32_SFLOAT;
	case MTLPixelFormatRGB10A2Unorm: return TF_A2R10G10B10_UNORM;
	case MTLPixelFormatRG8Unorm: return TF_R8G8_UNORM;
	case MTLPixelFormatR8Unorm: return TF_R8_UNORM;
	case MTLPixelFormatETC2_RGB8: return TF_ETC2_R8G8B8_UNORM;
	case MTLPixelFormatETC2_RGB8_sRGB: return TF_ETC2_R8G8B8_SRGB;
	case MTLPixelFormatETC2_RGB8A1: return TF_ETC2_R8G8B8A1_UNORM;
	case MTLPixelFormatETC2_RGB8A1_sRGB: return TF_ETC2_R8G8B8A1_SRGB;
	case MTLPixelFormatEAC_R11Unorm: return TF_EAC_R11_UNORM;
	case MTLPixelFormatEAC_R11Snorm: return TF_EAC_R11_SNORM;
	case MTLPixelFormatEAC_RG11Unorm: return TF_EAC_R11G11_UNORM;
	case MTLPixelFormatEAC_RG11Snorm: return TF_EAC_R11G11_SNORM;
	case MTLPixelFormatDepth32Float: return TF_D32_SFLOAT;
	case MTLPixelFormatInvalid: return TF_INVALID;
#if TARGET_OS_OSX
	case MTLPixelFormatBC5_RGUnorm: return TF_BC5_UNORM;
	case MTLPixelFormatBC5_RGSnorm: return TF_BC5_SNORM;
	case MTLPixelFormatBC6H_RGBUfloat: return TF_BC6H_UF16;
	case MTLPixelFormatBC6H_RGBFloat: return TF_BC6H_SF16;
	case MTLPixelFormatBC7_RGBAUnorm: return TF_BC7_UNORM;
	case MTLPixelFormatBC7_RGBAUnorm_sRGB: return TF_BC7_SRGB;
	case MTLPixelFormatDepth24Unorm_Stencil8: return TF_D24_STENCIL8;
#endif
	default: return TF_INVALID;
	}
}

static inline MTLBlendOperation
NeToMTLBlendOperation(enum BlendOperation op)
{
	switch (op) {
	case RE_BOP_ADD: return MTLBlendOperationAdd;
	case RE_BOP_SUBTRACT: return MTLBlendOperationSubtract;
	case RE_BOP_REVERSE_SUBTRACT: return MTLBlendOperationReverseSubtract;
	case RE_BOP_MIN: return MTLBlendOperationMin;
	case RE_BOP_MAX: return MTLBlendOperationMax;
	}
}

static inline MTLBlendFactor
NeToMTLBlendFactor(enum BlendFactor bf)
{
	switch (bf) {
	case RE_BF_ZERO: return MTLBlendFactorZero;
	case RE_BF_ONE: return MTLBlendFactorOne;
	case RE_BF_SRC_COLOR: return MTLBlendFactorSourceColor;
	case RE_BF_ONE_MINUS_SRC_COLOR: return MTLBlendFactorOneMinusSourceColor;
	case RE_BF_DST_COLOR: return MTLBlendFactorDestinationColor;
	case RE_BF_ONE_MINUS_DST_COLOR: return MTLBlendFactorOneMinusDestinationColor;
	case RE_BF_SRC_ALPHA: return MTLBlendFactorSourceAlpha;
	case RE_BF_ONE_MINUS_SRC_ALPHA: return MTLBlendFactorOneMinusSourceAlpha;
	case RE_BF_DST_ALPHA: return MTLBlendFactorDestinationAlpha;
	case RE_BF_ONE_MINUS_DST_ALPHA: return MTLBlendFactorOneMinusDestinationAlpha;
	case RE_BF_CONSTANT_COLOR: return MTLBlendFactorBlendColor;
	case RE_BF_ONE_MINUS_CONSTANT_COLOR: return MTLBlendFactorOneMinusBlendColor;
	case RE_BF_CONSTANT_ALPHA: return MTLBlendFactorBlendAlpha;
	case RE_BF_ONE_MINUS_CONSTANT_ALPHA: return MTLBlendFactorOneMinusBlendAlpha;
	case RE_BF_SRC_ALPHA_SATURATE: return MTLBlendFactorSourceAlphaSaturated;
	case RE_BF_SRC1_COLOR: return MTLBlendFactorSource1Color;
	case RE_BF_ONE_MINUS_SRC1_COLOR: return MTLBlendFactorOneMinusSource1Color;
	case RE_BF_SRC1_ALPHA: return MTLBlendFactorSource1Alpha;
	case RE_BF_ONE_MINUS_SRC1_ALPHA: return MTLBlendFactorOneMinusSource1Alpha;
	}
}

static inline MTLSamplerMinMagFilter
NeToMTLTextureFilter(enum ImageFilter tf)
{
	switch (tf) {
	case IF_NEAREST: return MTLSamplerMinMagFilterNearest;
	case IF_LINEAR: return MTLSamplerMinMagFilterLinear;
	case IF_CUBIC: return MTLSamplerMinMagFilterLinear;
	}
}

static inline MTLSamplerMipFilter
NeToMTLMipFilter(enum SamplerMipmapMode tmm)
{
	switch (tmm) {
	case SMM_NEAREST: return MTLSamplerMipFilterNearest;
	case SMM_LINEAR: return MTLSamplerMipFilterLinear;
	}
}

static inline MTLSamplerAddressMode
NeToMTLSamplerAddressMode(enum SamplerAddressMode tam)
{
	switch (tam) {
	case SAM_REPEAT: return MTLSamplerAddressModeRepeat;
	case SAM_MIRRORED_REPEAT: return MTLSamplerAddressModeMirrorRepeat;
	case SAM_CLAMP_TO_EDGE: return MTLSamplerAddressModeClampToEdge;
	case SAM_CLAMP_TO_BORDER: return MTLSamplerAddressModeClampToBorderColor;
	case SAM_MIRROR_CLAMP_TO_EDGE: return MTLSamplerAddressModeMirrorClampToEdge;
	}
}

static inline MTLCompareFunction
NeToMTLSamplerCompareFunction(enum CompareOperation co)
{
	switch (co) {
	case CO_NEVER: return MTLCompareFunctionNever;
	case CO_LESS: return MTLCompareFunctionLess;
	case CO_EQUAL: return MTLCompareFunctionEqual;
	case CO_LESS_EQUAL: return MTLCompareFunctionLessEqual;
	case CO_GREATER: return MTLCompareFunctionGreater;
	case CO_NOT_EQUAL: return MTLCompareFunctionNotEqual;
	case CO_GREATER_EQUAL: return MTLCompareFunctionGreaterEqual;
	case CO_ALWAYS: return MTLCompareFunctionAlways;
	}
}

static inline MTLSamplerBorderColor
NeToMTLSamplerBorderColor(enum BorderColor bc)
{
	switch (bc) {
	case BC_INT_TRANSPARENT_BLACK:
	case BC_FLOAT_TRANSPARENT_BLACK:
		return MTLSamplerBorderColorTransparentBlack;
	case BC_INT_OPAQUE_BLACK:
	case BC_FLOAT_OPAQUE_BLACK:
		return MTLSamplerBorderColorOpaqueBlack;
	case BC_INT_OPAQUE_WHITE:
	case BC_FLOAT_OPAQUE_WHITE:
		return MTLSamplerBorderColorOpaqueWhite;
	}
}

#endif /* _MTLDRIVER_H_ */
