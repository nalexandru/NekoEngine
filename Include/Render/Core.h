#ifndef _NE_RENDER_CORE_H_
#define _NE_RENDER_CORE_H_

#include <Engine/Types.h>
#include <Render/Device.h>
#include <Render/Context.h>

// Buffer
struct NeBufferDesc
{
	uint64_t size;
	enum NeBufferUsage usage;
	enum NeGPUMemoryType memoryType;
	const char *name;
};

struct NeBufferCreateInfo
{
	struct NeBufferDesc desc;
	void *data;
	uint64_t dataSize;
	bool keepData, dedicatedAllocation;
};

bool Re_CreateBuffer(const struct NeBufferCreateInfo *bci, NeBufferHandle *handle);
void Re_UpdateBuffer(NeBufferHandle handle, uint64_t offset, uint8_t *data, uint64_t size);
void Re_CmdCopyBuffer(NeBufferHandle src, uint64_t srcOffset, NeBufferHandle dst, uint64_t dstOffset, uint64_t size);
void *Re_MapBuffer(NeBufferHandle handle);
void Re_FlushBuffer(NeBufferHandle handle, uint64_t offset, uint64_t size);
void Re_UnmapBuffer(NeBufferHandle handle);
uint64_t Re_BufferAddress(NeBufferHandle handle, uint64_t offset);
const struct NeBufferDesc *Re_BufferDesc(NeBufferHandle handle);
void Re_DestroyBuffer(NeBufferHandle handle);

bool Re_ReserveBufferId(NeBufferHandle *handle);
void Re_ReleaseBufferId(NeBufferHandle handle);

bool Re_InitBufferSystem(void);
void Re_TermBufferSystem(void);

// Texture
#define RE_MAX_TEXTURES	65500

struct NeTextureDesc
{
	uint32_t width, height, depth;
	enum NeTextureType type;
	enum NeTextureUsage usage;
	enum NeTextureFormat format;
	uint32_t arrayLayers, mipLevels;
	bool gpuOptimalTiling;
	enum NeGPUMemoryType memoryType;
	const char *name;
};

struct NeTextureCreateInfo
{
	struct NeTextureDesc desc;
	void *data;
	uint64_t dataSize;
	bool keepData;
};

struct NeTextureResource
{
	uint16_t id;
	struct NeTexture *texture;
	struct NeTextureDesc desc;
};

const struct NeTextureDesc *Re_TextureDesc(NeTextureHandle tex);
enum NeTextureLayout Re_TextureLayout(NeTextureHandle tex);

void Re_CmdCopyBufferToTexture(NeBufferHandle src, struct NeTexture *dst, const struct NeBufferImageCopy *bic);
void Re_CmdCopyTextureToBuffer(struct NeTexture *src, NeBufferHandle dst, const struct NeBufferImageCopy *bic);

bool Re_InitTextureSystem(void);
void Re_TermTextureSystem(void);

// Sampler
struct NeSamplerDesc
{
	enum NeImageFilter minFilter, magFilter;
	enum NeSamplerMipmapMode mipmapMode;
	enum NeSamplerAddressMode addressModeU, addressModeV, addressModeW;
	bool enableAnisotropy;
	float maxAnisotropy;
	float minLod, maxLod, lodBias;
	bool unnormalizedCoordinates;
	bool enableCompare;
	enum NeCompareOperation compareOp;
	uint32_t borderColor;
	const char *name;
};

#define RE_SAMPLER_LOD_CLAMP_NONE	1000.f

struct NeSampler *Re_CreateSampler(const struct NeSamplerDesc *desc);
void Re_DestroySampler(struct NeSampler *s);

// Framebuffer
struct NeFramebufferAttachmentDesc
{
	enum NeTextureFormat format;
	enum NeTextureUsage usage;
};

struct NeFramebufferDesc
{
	uint32_t width, height, layers;
	struct NeFramebufferAttachmentDesc *attachments;
	uint32_t attachmentCount;
	struct NeRenderPassDesc *renderPassDesc;
	const char *name;
};

struct NeFramebuffer *Re_CreateFramebuffer(const struct NeFramebufferDesc *desc);
void Re_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex);
void Re_DestroyFramebuffer(struct NeFramebuffer *fb);

// Shader
struct NeShaderStageDesc
{
	enum NeShaderStage stage;
	void *module;
};

struct NeShaderStageInfo
{
	uint32_t stageCount;
	struct NeShaderStageDesc *stages;
};

struct NeShader
{
	uint64_t hash;
	enum NeShaderType type;

	union {
		struct {
			struct NeShaderStageInfo opaqueStages, transparentStages;
		};
		struct {
			uint32_t stageCount;
			struct NeShaderStageDesc *stages;
		};
	};

	char name[256];
};

bool Re_LoadShaders(void);
void Re_UnloadShaders(void);

struct NeShader *Re_GetShader(const char *name);
void *Re_ShaderModule(const char *name);

// Swapchain
extern struct NeSwapchain *Re_swapchain;

#define RE_INVALID_IMAGE	(void *)UINT64_MAX

struct NeSwapchain *Re_CreateSwapchain(struct NeSurface *surface, bool vsync);
void *Re_AcquireNextImage(struct NeSwapchain *swapchain);
bool Re_Present(struct NeSwapchain *swapchain, void *image, struct NeSemaphore *wait);
enum NeTextureFormat Re_SwapchainFormat(struct NeSwapchain *swapchain);
struct NeTexture *Re_SwapchainTexture(struct NeSwapchain *swapchain, void *image);
void Re_SwapchainDesc(struct NeSwapchain *swapchain, struct NeFramebufferAttachmentDesc *desc);
void Re_DestroySwapchain(struct NeSwapchain *swapchain);

// Pipeline
#define RE_TOPOLOGY_POINTS			((uint64_t)  0 <<  0)
#define RE_TOPOLOGY_LINES			((uint64_t)  1 <<  0)
#define RE_TOPOLOGY_TRIANGLES		((uint64_t)  2 <<  0)
#define RE_TOPOLOGY_BITS			((uint64_t)  3 <<  0)
#define RE_TOPOLOGY_OFFSET			0

#define RE_POLYGON_FILL				((uint64_t)  0 <<  2)
#define RE_POLYGON_LINE				((uint64_t)  1 <<  2)
#define RE_POLYGON_POINT			((uint64_t)  2 <<  2)
#define RE_POLYGON_BITS				((uint64_t)  3 <<  2)
#define RE_POLYGON_OFFSET			2

#define RE_CULL_NONE				((uint64_t)  0 <<  4)
#define RE_CULL_BACK				((uint64_t)  1 <<  4)
#define RE_CULL_FRONT				((uint64_t)  2 <<  4)
#define RE_CULL_FRONT_AND_BACK		((uint64_t)  3 <<  4)
#define RE_CULL_BITS				((uint64_t)  3 <<  4)
#define RE_CULL_OFFSET				4

#define RE_FRONT_FACE_CCW			((uint64_t)  0 <<  6)
#define RE_FRONT_FACE_CW			((uint64_t)  1 <<  6)
#define RE_FRONT_FACE_BITS			((uint64_t)  1 <<  6)
#define RE_FRONT_FACE_OFFSET		6

#define RE_DISCARD					((uint64_t)  1 <<  7)
#define RE_DEPTH_TEST				((uint64_t)  1 <<  8)
#define RE_DEPTH_WRITE				((uint64_t)  1 <<  9)
#define RE_DEPTH_BIAS				((uint64_t)  1 << 10)
#define RE_DEPTH_CLAMP				((uint64_t)  1 << 11)
#define RE_DEPTH_BOUNDS				((uint64_t)  1 << 12)

#define RE_DEPTH_OP_NEVER			((uint64_t)  0 << 13)
#define RE_DEPTH_OP_LESS			((uint64_t)  1 << 13)
#define RE_DEPTH_OP_EQUAL			((uint64_t)  2 << 13)
#define RE_DEPTH_OP_LESS_EQUAL		((uint64_t)  3 << 13)
#define RE_DEPTH_OP_GREATER			((uint64_t)  4 << 13)
#define RE_DEPTH_OP_NOT_EQUAL		((uint64_t)  5 << 13)
#define RE_DEPTH_OP_GREATER_EQUAL	((uint64_t)  6 << 13)
#define RE_DEPTH_OP_ALWAYS			((uint64_t)  7 << 13)
#define RE_DEPTH_OP_BITS			((uint64_t)  7 << 13)
#define RE_DEPTH_OP_OFFSET			13

#define RE_MULTISAMPLE				((uint64_t)  1 << 16)
#define RE_SAMPLE_SHADING			((uint64_t)  1 << 17)
#define RE_ALPHA_TO_COVERAGE		((uint64_t)  1 << 18)
#define RE_ALPHA_TO_ONE				((uint64_t)  1 << 19)
#define RE_MS_2_SAMPLES				((uint64_t)  0 << 20)
#define RE_MS_4_SAMPLES				((uint64_t)  1 << 20)
#define RE_MS_8_SAMPLES				((uint64_t)  2 << 20)
#define RE_MS_16_SAMPLES			((uint64_t)  3 << 20)
#define RE_SAMPLES_BITS				((uint64_t)  3 << 20)
#define RE_SAMPLES_OFFSET			20

#define RE_WRITE_MASK_R				0x00000001
#define RE_WRITE_MASK_G				0x00000002
#define RE_WRITE_MASK_B				0x00000004
#define RE_WRITE_MASK_A				0x00000008
#define RE_WRITE_MASK_RGB			RE_WRITE_MASK_R | RE_WRITE_MASK_G | RE_WRITE_MASK_B
#define RE_WRITE_MASK_RGBA			RE_WRITE_MASK_RGB | RE_WRITE_MASK_A

struct NeBlendAttachmentDesc
{
	bool enableBlend;
	enum NeBlendFactor srcColor;
	enum NeBlendFactor dstColor;
	enum NeBlendOperation colorOp;
	enum NeBlendFactor srcAlpha;
	enum NeBlendFactor dstAlpha;
	enum NeBlendOperation alphaOp;
	int32_t writeMask;
};
// TODO: is independent blend supported in Metal/D3D12 ?

enum NeVertexInputRate
{
	VIR_VERTEX		= 0,
	VIR_INSTANCE	= 1
};

enum NeVertexFormat
{
	VF_FLOAT,
	VF_FLOAT2,
	VF_FLOAT3,
	VF_FLOAT4
};

struct NeVertexAttribute
{
	uint32_t location;
	uint32_t binding;
	enum NeVertexFormat format;
	uint32_t offset;
};

struct NeVertexBinding
{
	uint32_t binding;
	uint32_t stride;
	enum NeVertexInputRate inputRate;
};

struct NeVertexDesc
{
	struct NeVertexAttribute *attributes;
	uint32_t attributeCount;
	struct NeVertexBinding *bindings;
	uint32_t bindingCount;
};

struct NeGraphicsPipelineDesc
{
	uint64_t flags;
	struct NeVertexDesc vertexDesc;
	struct NeShaderStageInfo *stageInfo;
	struct NeRenderPassDesc *renderPassDesc;
	uint32_t pushConstantSize;
	uint32_t attachmentCount;
	const struct NeBlendAttachmentDesc *attachments;
	enum NeTextureFormat depthFormat;
	const char *name;
};

struct NeComputePipelineDesc
{
	struct NeShaderStageInfo *stageInfo;
	struct {
		uint32_t x;
		uint32_t y;
		uint32_t z;
	} threadsPerThreadgroup;
	uint32_t pushConstantSize;
	const char *name;
};

struct NeRayTracingPipelineDesc
{
	struct NeShaderStageInfo *stageInfo;
};

struct NePipeline *Re_GraphicsPipeline(const struct NeGraphicsPipelineDesc *desc);
struct NePipeline *Re_ComputePipeline(const struct NeComputePipelineDesc *desc);
struct NePipeline *Re_RayTracingPipeline(struct NeShaderBindingTable *sbt, uint32_t maxDepth);

bool Re_InitPipelines(void);
void Re_TermPipelines(void);

void Re_LoadPipelineCache(void);
void Re_SavePipelineCache(void);

// Synchronization
struct NeMemoryBarrier
{
	enum NePipelineAccess srcAccess;
	enum NePipelineAccess dstAccess;
};

struct NeBufferBarrier
{
	enum NePipelineAccess srcAccess;
	enum NePipelineAccess dstAccess;
	enum NeRenderQueue srcQueue;
	enum NeRenderQueue dstQueue;
	struct NeBuffer *buffer;
	uint64_t offset;
	uint64_t size;
};

struct NeImageBarrier
{
	enum NePipelineAccess srcAccess;
	enum NePipelineAccess dstAccess;
	enum NeTextureLayout oldLayout;
	enum NeTextureLayout newLayout;
	enum NeRenderQueue srcQueue;
	enum NeRenderQueue dstQueue;
	struct NeTexture *texture;
	struct NeImageSubresource subresource;
};

struct NeSemaphore *Re_CreateSemaphore(void);
bool Re_WaitSemaphore(struct NeSemaphore *s, uint64_t value, uint64_t timeout);
bool Re_WaitSemaphores(uint32_t count, struct NeSemaphore *s, uint64_t *values, uint64_t timeout);
bool Re_SignalSemaphore(struct NeSemaphore *s, uint64_t value);
void Re_DestroySemaphore(struct NeSemaphore *s);

struct NeFence *Re_CreateFence(bool createSignaled);
bool Re_WaitForFence(struct NeFence *f, uint64_t timeout);
void Re_DestroyFence(struct NeFence *f);

// RenderPass
struct NeAttachmentDesc
{
	bool mayAlias;
	enum NeTextureFormat format;
	enum NeAttachmentLoadOp loadOp;
	enum NeAttachmentStoreOp storeOp;
	enum NeAttachmentSampleCount samples;
	enum NeTextureLayout layout, initialLayout, finalLayout;
	union {
		float clearColor[4];
		struct {
			float clearDepth;
			uint8_t clearStencil;
		};
	};
};

struct NeRenderPassDesc *Re_CreateRenderPassDesc(const struct NeAttachmentDesc *attachments, uint32_t count, const struct NeAttachmentDesc *depthAttachment, const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount);
void Re_DestroyRenderPassDesc(struct NeRenderPassDesc *pass);

// Direct I/O
NeDirectIOHandle Re_OpenFile(const char *path);
void Re_CloseFile(NeDirectIOHandle handle);

// Interface
struct NeRenderInterface *Re_CreateRenderInterface(void);
void Re_DestroyRenderInterface(struct NeRenderInterface *iface);

#endif /* _NE_RENDER_DRIVER_CORE_H_ */
