#ifndef _RE_CORE_H_
#define _RE_CORE_H_

#include <Engine/Types.h>
#include <Render/Driver/Device.h>
#include <Render/Driver/Context.h>

// Buffer
struct BufferDesc
{
	uint64_t size;
	enum BufferUsage usage;
	enum GPUMemoryType memoryType;
};

struct BufferCreateInfo
{
	struct BufferDesc desc;
	void *data;
	uint64_t dataSize;
	bool keepData, dedicatedAllocation;
};

bool Re_CreateBuffer(const struct BufferCreateInfo *bci, BufferHandle *handle);
void Re_UpdateBuffer(BufferHandle handle, uint64_t offset, uint8_t *data, uint64_t size);
void Re_CmdCopyBuffer(BufferHandle src, uint64_t srcOffset, BufferHandle dst, uint64_t dstOffset, uint64_t size);
void *Re_MapBuffer(BufferHandle handle);
void Re_UnmapBuffer(BufferHandle handle);
uint64_t Re_BufferAddress(BufferHandle handle, uint64_t offset);
const struct BufferDesc *Re_BufferDesc(BufferHandle handle);
void Re_DestroyBuffer(BufferHandle handle);

bool Re_InitBufferSystem(void);
void Re_TermBufferSystem(void);

// Texture
struct TextureDesc
{
	uint32_t width, height, depth;
	enum TextureType type;
	enum TextureUsage usage;
	enum TextureFormat format;
	uint32_t arrayLayers, mipLevels;
	bool gpuOptimalTiling;
	enum GPUMemoryType memoryType;
};

struct TextureCreateInfo
{
	struct TextureDesc desc;
	void *data;
	uint64_t dataSize;
	bool keepData;
};

struct TextureResource
{
	uint16_t id;
	struct Texture *texture;
	struct TextureDesc desc;
};

bool Re_CreateTextureResource(const char *name, const struct TextureCreateInfo *ci, struct TextureResource *tex, Handle h);
bool Re_LoadTextureResource(struct ResourceLoadInfo *li, const char *args, struct TextureResource *tex, Handle h);
void Re_UnloadTextureResource(struct TextureResource *tex, Handle h);

const struct TextureDesc *Re_TextureDesc(TextureHandle tex);
enum TextureLayout Re_TextureLayout(TextureHandle tex);

//static inline void Re_UpdateTexture(struct Texture *tex, uint64_t offset, uint64_t size, void *data);

bool Re_InitTextureSystem(void);
void Re_TermTextureSystem(void);

// Sampler
struct SamplerDesc
{
	enum ImageFilter minFilter, magFilter;
	enum SamplerMipmapMode mipmapMode;
	enum SamplerAddressMode addressModeU, addressModeV, addressModeW;
	bool enableAnisotropy;
	float maxAnisotropy;
	float minLod, maxLod, lodBias;
	bool unnormalizedCoordinates;
	bool enableCompare;
	enum CompareOperation compareOp;
	uint32_t borderColor;
};

static inline struct Sampler *Re_CreateSampler(const struct SamplerDesc *desc) { return Re_deviceProcs.CreateSampler(Re_device, desc); }
static inline void Re_DestroySampler(struct Sampler *s) { Re_deviceProcs.DestroySampler(Re_device, s); }

// Framebuffer
struct FramebufferAttachmentDesc
{
	enum TextureFormat format;
	enum TextureUsage usage;
};

struct FramebufferDesc
{
	uint32_t width, height, layers;
	struct FramebufferAttachmentDesc *attachments;
	uint32_t attachmentCount;
	struct RenderPassDesc *renderPassDesc;
};

static inline struct Framebuffer *Re_CreateFramebuffer(const struct FramebufferDesc *desc) { return Re_deviceProcs.CreateFramebuffer(Re_device, desc); }
static inline void Re_SetAttachment(struct Framebuffer *fb, uint32_t pos, struct Texture *tex) { Re_deviceProcs.SetAttachment(fb, pos, tex); }
static inline void Re_DestroyFramebuffer(struct Framebuffer *fb) { Re_deviceProcs.DestroyFramebuffer(Re_device, fb); }

// Shader
struct ShaderStageDesc
{
	enum ShaderStage stage;
	void *module;
};

struct Shader
{
	uint64_t hash;
	enum ShaderType type;
	uint32_t stageCount;
	struct ShaderStageDesc *stages;
	char name[256];
};

bool Re_LoadShaders(void);
void Re_UnloadShaders(void);

struct Shader *Re_GetShader(const char *name);

// Swapchain
extern struct Swapchain *Re_swapchain;

#define RE_INVALID_IMAGE	(void *)UINT64_MAX

static inline struct Swapchain *Re_CreateSwapchain(struct Surface *surface) { return Re_deviceProcs.CreateSwapchain(Re_device, surface); }
static inline void *Re_AcquireNextImage(struct Swapchain *swapchain) { return Re_deviceProcs.AcquireNextImage(Re_device, swapchain); }
static inline bool Re_Present(struct Swapchain *swapchain, void *image) { return Re_deviceProcs.Present(Re_device, Re_CurrentContext(), swapchain, image); }
static inline enum TextureFormat Re_SwapchainFormat(struct Swapchain *swapchain) { return Re_deviceProcs.SwapchainFormat(swapchain); }
static inline struct Texture *Re_SwapchainTexture(struct Swapchain *swapchain, void *image) { return Re_deviceProcs.SwapchainTexture(swapchain, image); }
static inline void Re_DestroySwapchain(struct Swapchain *swapchain) { Re_deviceProcs.DestroySwapchain(Re_device, swapchain); }

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

#define RE_MULTISAMPLE				((uint64_t)  1 << 15)
#define RE_SAMPLE_SHADING			((uint64_t)  1 << 16)
#define RE_ALPHA_TO_COVERAGE		((uint64_t)  1 << 17)
#define RE_ALPHA_TO_ONE				((uint64_t)  1 << 18)
#define RE_MS_2_SAMPLES				((uint64_t)  0 << 19)
#define RE_MS_4_SAMPLES				((uint64_t)  1 << 19)
#define RE_MS_8_SAMPLES				((uint64_t)  2 << 19)
#define RE_MS_16_SAMPLES			((uint64_t)  3 << 19)
#define RE_SAMPLES_BITS				((uint64_t)  3 << 19)
#define RE_SAMPLES_OFFSET			19

#define RE_WRITE_MASK_R				0x00000001
#define RE_WRITE_MASK_G				0x00000002
#define RE_WRITE_MASK_B				0x00000004
#define RE_WRITE_MASK_A				0x00000008
#define RE_WRITE_MASK_RGB			RE_WRITE_MASK_R | RE_WRITE_MASK_G | RE_WRITE_MASK_B
#define RE_WRITE_MASK_RGBA			RE_WRITE_MASK_RGB | RE_WRITE_MASK_A

struct BlendAttachmentDesc
{
	bool enableBlend;
	enum BlendFactor srcColor;
	enum BlendFactor dstColor;
	enum BlendOperation colorOp;
	enum BlendFactor srcAlpha;
	enum BlendFactor dstAlpha;
	enum BlendOperation alphaOp;
	int32_t writeMask;
};
// TODO: is independent blend supported in Metal/D3D12 ?

struct GraphicsPipelineDesc
{
	uint64_t flags;
	struct Shader *shader;
	struct RenderPassDesc *renderPassDesc;
	uint32_t pushConstantSize;
	uint32_t attachmentCount;
	const struct BlendAttachmentDesc *attachments;
};

struct Pipeline *Re_GraphicsPipeline(const struct GraphicsPipelineDesc *desc);
struct Pipeline *Re_ComputePipeline(struct Shader *sh);
struct Pipeline *Re_RayTracingPipeline(struct ShaderBindingTable *sbt, uint32_t maxDepth);

bool Re_InitPipelines(void);
void Re_TermPipelines(void);

#endif /* _RE_CORE_H_ */
