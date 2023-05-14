#ifndef _MTLBACKEND_H_
#define _MTLBACKEND_H_

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

#include <System/Log.h>
#include <System/Memory.h>
#include <Render/Types.h>
#include <Render/Render.h>
#include <Render/Backend.h>
#include <Engine/Config.h>
#include <Engine/Job.h>

#define MTLDRV_MOD	L"MetalDriver"

#define PS_RENDER		0
#define PS_COMPUTE		1
#define PS_RAY_TRACING	2
struct NePipeline
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

struct NeTexture
{
	id<MTLTexture> tex;
	enum NeTextureLayout layout;
};

struct NeBuffer
{
	id<MTLBuffer> buff;
	enum NeGPUMemoryType memoryType;
	uint32_t location;
};

struct NeRenderDevice
{
	id<MTLDevice> dev;
};

extern id<MTLDevice> MTL_device;

#define RC_RENDER	0
#define RC_COMPUTE	1
#define RC_BLIT		2
struct NeRenderContext
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
			struct NePipeline *boundPipeline;
			struct {
				MTLIndexType type;
				struct NeBuffer *buffer;
				uint64_t offset;
			} boundIndexBuffer;
		};
		MTLSize threadsPerThreadgroup;
	};
	struct NeArray submitted;
	struct NeArray resourceBarriers;
	bool scopeBarrier;
	id<MTLParallelRenderCommandEncoder> parallelEncoder;
	id<MTLIOCommandQueue> ioQueue API_AVAILABLE(macosx(13), ios(16));
	id<MTLIOCommandBuffer> ioCmdBuffer API_AVAILABLE(macosx(13), ios(16));
};

struct NeFramebuffer
{
	id<MTLTexture> *attachments;
	uint32_t attachmentCount;
};

struct NeRenderPassDesc
{
	MTLRenderPassDescriptor *desc;
	uint32_t colorAttachments, inputAttachments;
	MTLPixelFormat* attachmentFormats;
	MTLPixelFormat depthFormat;
};

struct NeAccelerationStructure
{
	id<MTLAccelerationStructure> as;
	MTLAccelerationStructureDescriptor *desc;
};

struct NeSemaphore
{
	id<MTLEvent> event;
	uint64_t value;
};

struct NeSwapchain
{
	CAMetalLayer *layer;
	id<MTLEvent> event;
	uint64_t value;
};

struct Mtld_SubmitInfo
{
	id<MTLEvent> signal;
	uint64_t signalValue;
	id<MTLCommandBuffer> cmdBuffer;
};

extern dispatch_semaphore_t MTL_frameSemaphore;

// Device
struct NeRenderDevice *MTL_CreateDevice(struct NeRenderDeviceInfo *info,
									  struct NeRenderDeviceProcs *devProcs,
									  struct NeRenderContextProcs *ctxProcs);
bool MTL_Execute(id<MTLDevice> dev, struct NeRenderContext *ctx, bool wait);
void MTL_WaitIdle(id<MTLDevice> dev);
void MTL_DestroyDevice(id<MTLDevice> dev);

// Pipeline
struct NePipeline *MTL_GraphicsPipeline(id<MTLDevice> dev, const struct NeGraphicsPipelineDesc *gpDesc);
struct NePipeline *MTL_ComputePipeline(id<MTLDevice> dev, const struct NeComputePipelineDesc *cpDesc);
struct NePipeline *MTL_RayTracingPipeline(id<MTLDevice> dev, struct NeShaderBindingTable *sbt, uint32_t maxDepth);
void MTL_LoadPipelineCache(id<MTLDevice> dev);
void MTL_SavePipelineCache(id<MTLDevice> dev);
void MTL_DestroyPipeline(id<MTLDevice> dev, struct NePipeline *p);

// Surface
void *MTL_CreateSurface(id<MTLDevice> dev, WINDOWTYPE *window);
void MTL_DestroySurface(id<MTLDevice> dev, VIEWTYPE *view);

// Swapchain
struct NeSwapchain *MTL_CreateSwapchain(id<MTLDevice> dev, VIEWTYPE *view, bool verticalSync);
void MTL_DestroySwapchain(id<MTLDevice> dev, struct NeSwapchain *);
void *MTL_AcquireNextImage(id<MTLDevice> dev, struct NeSwapchain *);
bool MTL_Present(id<MTLDevice> dev, struct NeRenderContext *ctx, struct NeSwapchain *sw, id<CAMetalDrawable> image, id<MTLFence> f);
struct NeTexture *MTL_SwapchainTexture(struct NeSwapchain *sw, id<CAMetalDrawable> image);
enum NeTextureFormat MTL_SwapchainFormat(struct NeSwapchain *sw);
void MTL_SwapchainDesc(struct NeSwapchain *sw, struct NeFramebufferAttachmentDesc *desc);
void MTL_ScreenResized(id<MTLDevice> dev, struct NeSwapchain *sw);

// Context
void MTL_InitContextProcs(struct NeRenderContextProcs *p);
struct NeRenderContext *MTL_CreateContext(id<MTLDevice> dev);
void MTL_ResetContext(id<MTLDevice> dev, struct NeRenderContext *ctx);
void MTL_DestroyContext(id<MTLDevice> dev, struct NeRenderContext *ctx);

// Texture
MTLTextureDescriptor *MTL_TextureDescriptor(id<MTLDevice> dev, const struct NeTextureDesc *desc);
struct NeTexture *MTL_CreateTexture(id<MTLDevice> dev, const struct NeTextureDesc *desc, uint16_t location);
enum NeTextureLayout MTL_TextureLayout(const struct NeTexture *tex);
void MTL_DestroyTexture(id<MTLDevice> dev, struct NeTexture *tex);

// Buffer
struct NeBuffer *MTL_CreateBuffer(id<MTLDevice> dev, const struct NeBufferDesc *desc, uint16_t location);
void MTL_UpdateBuffer(id<MTLDevice> dev, struct NeBuffer *buff, uint64_t offset, uint8_t *data, uint64_t size);
void *MTL_MapBuffer(id<MTLDevice> dev, struct NeBuffer *buff);
void MTL_FlushBuffer(id<MTLDevice> dev, struct NeBuffer *buff, uint64_t offset, uint64_t size);
void MTL_UnmapBuffer(id<MTLDevice> dev, struct NeBuffer *buff);
uint64_t MTL_BufferAddress(id<MTLDevice> dev, const struct NeBuffer *buff, uint64_t offset);
uint64_t MTL_OffsetAddress(uint64_t address, uint64_t offset);
void MTL_DestroyBuffer(id<MTLDevice> dev, struct NeBuffer *buff);

// Acceleration Structure
struct NeAccelerationStructure *MTL_CreateAccelerationStructure(id<MTLDevice> dev, const struct NeAccelerationStructureCreateInfo *asci);
uint64_t MTL_AccelerationStructureHandle(id<MTLDevice> dev, const struct NeAccelerationStructure *as);
void MTL_DestroyAccelerationStructure(id<MTLDevice> dev, struct NeAccelerationStructure *as);

// Framebuffer
struct NeFramebuffer *MTL_CreateFramebuffer(id<MTLDevice> dev, const struct NeFramebufferDesc *desc);
void MTL_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex);
const struct NeFramebufferDesc *MTL_FramebufferDesc(const struct NeFramebuffer *fb);
void MTL_DestroyFramebuffer(id<MTLDevice> dev, struct NeFramebuffer *fb);

// Render Pass
struct NeRenderPassDesc *MTL_CreateRenderPassDesc(id<MTLDevice> dev, const struct NeAttachmentDesc *attachments, uint32_t count, const struct NeAttachmentDesc *depthAttachment,
												const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount);
void MTL_DestroyRenderPassDesc(id<MTLDevice> dev, struct NeRenderPassDesc *pass);

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
id<MTLSamplerState> MTL_CreateSampler(id<MTLDevice> dev, const struct NeSamplerDesc *sDesc);
void MTL_DestroySampler(id<MTLDevice> dev, id<MTLSamplerState> s);

// Transient Resources
struct NeTexture *MTL_CreateTransientTexture(id<MTLDevice> dev, const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
struct NeBuffer *MTL_CreateTransientBuffer(id<MTLDevice> dev, const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
bool MTL_InitTransientHeap(id<MTLDevice> dev, uint64_t size);
bool MTL_ResizeTransientHeap(id<MTLDevice> dev, uint64_t size);
void MTL_TermTransientHeap(id<MTLDevice> dev);

// Synchronization
struct NeSemaphore *MTL_CreateSemaphore(id<MTLDevice> dev);
bool MTL_WaitSemaphore(id<MTLDevice> dev, struct NeSemaphore *s, uint64_t value, uint64_t timeout);
bool MTL_WaitSemaphores(id<MTLDevice> dev, uint32_t count, struct NeSemaphore **s, uint64_t *values, uint64_t timeout);
bool MTL_SignalSemaphore(id<MTLDevice> dev, struct NeSemaphore *s, uint64_t value);
void MTL_DestroySemaphore(id<MTLDevice> dev, struct NeSemaphore *s);

dispatch_semaphore_t MTL_CreateFence(id<MTLDevice> dev, bool createSignaled);
void MTL_SignalFence(id<MTLDevice> dev, dispatch_semaphore_t ds);
bool MTL_WaitForFence(id<MTLDevice> dev, dispatch_semaphore_t ds, uint64_t timeout);
void MTL_DestroyFence(id<MTLDevice> dev, dispatch_semaphore_t ds);

// Memory
bool MTLBk_InitMemory(void);
id<MTLBuffer> MTLBk_CreateBuffer(id<MTLDevice> dev, uint64_t size, MTLResourceOptions options);
id<MTLTexture> MTLBk_CreateTexture(id<MTLDevice> dev, MTLTextureDescriptor *desc);
void MTLBk_SetRenderHeaps(id<MTLRenderCommandEncoder> encoder);
void MTLBk_SetComputeHeaps(id<MTLComputeCommandEncoder> encoder);
void MTLBk_TermMemory(void);

// Utility functions
static inline MTLResourceOptions
MTL_GPUMemoryTypetoResourceOptions(bool hasUnifiedMemory, enum NeGPUMemoryType type)
{
	MTLResourceOptions options = MTLResourceHazardTrackingModeTracked;
	
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
NeToMTLTextureFormat(enum NeTextureFormat fmt)
{
	switch (fmt) {
	case TF_R8G8B8A8_UNORM: return MTLPixelFormatRGBA8Unorm;
	case TF_R8G8B8A8_SRGB: return MTLPixelFormatRGBA8Unorm_sRGB;
	case TF_B8G8R8A8_UNORM: return MTLPixelFormatBGRA8Unorm;
	case TF_B8G8R8A8_SRGB: return MTLPixelFormatBGRA8Unorm_sRGB;
	case TF_R16G16B16A16_UNORM: return MTLPixelFormatRGBA16Unorm;
	case TF_R16G16B16A16_SFLOAT: return MTLPixelFormatRGBA16Float;
	case TF_R32G32B32A32_UINT: return MTLPixelFormatRGBA32Uint;
	case TF_R32G32B32A32_SFLOAT: return MTLPixelFormatRGBA32Float;
	case TF_A2R10G10B10_UNORM: return MTLPixelFormatRGB10A2Unorm;
	case TF_R8G8_UNORM: return MTLPixelFormatRG8Unorm;
	case TF_R16G16_UNORM: return MTLPixelFormatRG16Unorm;
	case TF_R32G32_UINT: return MTLPixelFormatRG32Uint;
	case TF_R8_UNORM: return MTLPixelFormatR8Unorm;
	case TF_R16_UNORM: return MTLPixelFormatR16Unorm;
	case TF_R32_UINT: return MTLPixelFormatR32Uint;
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

static inline enum NeTextureFormat
MTLToNeTextureFormat(MTLPixelFormat fmt)
{
	switch (fmt) {
	case MTLPixelFormatRGBA8Unorm: return TF_R8G8B8A8_UNORM;
	case MTLPixelFormatRGBA8Unorm_sRGB: return TF_R8G8B8A8_SRGB;
	case MTLPixelFormatBGRA8Unorm: return TF_B8G8R8A8_UNORM;
	case MTLPixelFormatBGRA8Unorm_sRGB: return TF_B8G8R8A8_SRGB;
	case MTLPixelFormatRGBA16Unorm: return TF_R16G16B16A16_UNORM;
	case MTLPixelFormatRGBA16Float: return TF_R16G16B16A16_SFLOAT;
	case MTLPixelFormatRGBA32Uint: return TF_R32G32B32A32_UINT;
	case MTLPixelFormatRGBA32Float: return TF_R32G32B32A32_SFLOAT;
	case MTLPixelFormatRGB10A2Unorm: return TF_A2R10G10B10_UNORM;
	case MTLPixelFormatRG8Unorm: return TF_R8G8_UNORM;
	case MTLPixelFormatRG16Unorm: return TF_R16G16_UNORM;
	case MTLPixelFormatRG32Uint: return TF_R32G32_UINT;
	case MTLPixelFormatR8Unorm: return TF_R8_UNORM;
	case MTLPixelFormatR16Unorm: return TF_R16_UNORM;
	case MTLPixelFormatR32Uint: return TF_R32_UINT;
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

static inline NSUInteger
MTL_TextureFormatSize(enum NeTextureFormat fmt)
{
	switch (fmt) {
	case TF_R8G8B8A8_UNORM:
	case TF_R8G8B8A8_SRGB:
	case TF_B8G8R8A8_UNORM:
	case TF_B8G8R8A8_SRGB: return 4;
	case TF_R16G16B16A16_UNORM:
	case TF_R16G16B16A16_SFLOAT: return 8;
	case TF_R32G32B32A32_SFLOAT: return 16;
	case TF_A2R10G10B10_UNORM: return 4;
	case TF_R8G8_UNORM: return 2;
	case TF_R8_UNORM: return 1;
	/*case TF_ETC2_R8G8B8_UNORM: return MTLPixelFormatETC2_RGB8;
	case TF_ETC2_R8G8B8_SRGB: return MTLPixelFormatETC2_RGB8_sRGB;
	case TF_ETC2_R8G8B8A1_UNORM: return MTLPixelFormatETC2_RGB8A1;
	case TF_ETC2_R8G8B8A1_SRGB: return MTLPixelFormatETC2_RGB8A1_sRGB;
	case TF_EAC_R11_UNORM: return MTLPixelFormatEAC_R11Unorm;
	case TF_EAC_R11_SNORM: return MTLPixelFormatEAC_R11Snorm;
	case TF_EAC_R11G11_UNORM: return MTLPixelFormatEAC_RG11Unorm;
	case TF_EAC_R11G11_SNORM: return MTLPixelFormatEAC_RG11Snorm;
	case TF_BC5_UNORM: return MTLPixelFormatBC5_RGUnorm;
	case TF_BC5_SNORM: return MTLPixelFormatBC5_RGSnorm;
	case TF_BC6H_UF16: return MTLPixelFormatBC6H_RGBUfloat;
	case TF_BC6H_SF16: return MTLPixelFormatBC6H_RGBFloat;
	case TF_BC7_UNORM: return MTLPixelFormatBC7_RGBAUnorm;
	case TF_BC7_SRGB: return MTLPixelFormatBC7_RGBAUnorm_sRGB;*/
	case TF_D24_STENCIL8: return 4;
	case TF_D32_SFLOAT: return 4;
	default: return UINT32_MAX;
	}
	
	return UINT32_MAX; // this will force a crash
}

static inline MTLVertexFormat
NeToMTLVertexFormat(enum NeVertexFormat fmt)
{
	switch (fmt) {
	case VF_FLOAT: return MTLVertexFormatFloat;
	case VF_FLOAT2: return MTLVertexFormatFloat2;
	case VF_FLOAT3: return MTLVertexFormatFloat3;
	case VF_FLOAT4: return MTLVertexFormatFloat4;
	}
}

static inline enum NeVertexFormat
MTLToNeVertexFormat(MTLVertexFormat fmt)
{
	switch (fmt) {
	case MTLVertexFormatFloat: return VF_FLOAT;
	case MTLVertexFormatFloat2: return VF_FLOAT2;
	case MTLVertexFormatFloat3: return VF_FLOAT3;
	case MTLVertexFormatFloat4: return VF_FLOAT4;
	default: return UINT32_MAX;
	}
}

static inline MTLBlendOperation
NeToMTLBlendOperation(enum NeBlendOperation op)
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
NeToMTLBlendFactor(enum NeBlendFactor bf)
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
NeToMTLTextureFilter(enum NeImageFilter tf)
{
	switch (tf) {
	case IF_NEAREST: return MTLSamplerMinMagFilterNearest;
	case IF_LINEAR: return MTLSamplerMinMagFilterLinear;
	case IF_CUBIC: return MTLSamplerMinMagFilterLinear;
	}
}

static inline MTLSamplerMipFilter
NeToMTLMipFilter(enum NeSamplerMipmapMode tmm)
{
	switch (tmm) {
	case SMM_NEAREST: return MTLSamplerMipFilterNearest;
	case SMM_LINEAR: return MTLSamplerMipFilterLinear;
	}
}

static inline MTLSamplerAddressMode
NeToMTLSamplerAddressMode(enum NeSamplerAddressMode tam)
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
NeToMTLSamplerCompareFunction(enum NeCompareOperation co)
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
NeToMTLSamplerBorderColor(enum NeBorderColor bc)
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

#endif /* _MTLBACKEND_H_ */

/* NekoEngine
 *
 * MTLBackend.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
