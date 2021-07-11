#ifndef _NE_RENDER_TYPES_H_
#define _NE_RENDER_TYPES_H_

#include <Engine/Types.h>

// Resource definitions
#define RES_MODEL				"Model"
#define RES_MATERIAL			"Material"
#define RES_TEXTURE				"Texture"
#define RES_ANIMATION_CLIP		"AnimationClip"

// Acceleration Structure

enum AccelerationStructureType
{
	AS_TOP_LEVEL,
	AS_BOTTOM_LEVEL
};

enum AccelerationStructureFlags
{
	ASF_ALLOW_UPDATE		= 0x00000001,
	ASF_ALLOW_COMPACTION	= 0x00000002,
	ASF_FAST_TRACE			= 0x00000004,
	ASF_FAST_BUILD			= 0x00000008,
	ASF_LOW_MEMORY			= 0x00000010
};

enum AccelerationStructureBuildMode
{
	ASB_BUILD	= 0,
	ASB_UPDATE	= 1
};

enum AccelerationStructureGeometryType
{
	ASG_TRIANGLES	= 0,
	ASG_AABBS		= 1,
	ASG_INSTANCES	= 2
};

enum AccelerationStructureGeometryFlags
{
	ASGF_OPAQUE								= 0x00000001,
	ASGF_NO_DUPLICATE_ANY_HIT_INVOCATION	= 0x00000002
};

struct AccelerationStructure;
struct AccelerationStructureAABB;
struct AccelerationStructureInstance;
struct AccelerationStructureGeometryDesc;
struct AccelerationStructureBuildInfo;
struct AccelerationStructureRangeInfo;
struct AccelerationStructureDesc;
struct AccelerationStructureCreateInfo;

// Buffer

enum BufferUsage
{
	BU_TRANSFER_SRC			= 0x00000001,
	BU_TRANSFER_DST			= 0x00000002,
	BU_UNIFORM_BUFFER		= 0x00000010,
	BU_STORAGE_BUFFER		= 0x00000020,
	BU_INDEX_BUFFER			= 0x00000040,
	BU_INDIRECT_BUFFER		= 0x00000100,
	BU_AS_BUILD_INPUT		= 0x00080000,
	BU_AS_STORAGE			= 0x00100000,
	BU_SHADER_BINDING_TABLE	= 0x00000400
};

struct Buffer;
struct BufferDesc;
struct BufferCreateInfo;
typedef uint16_t BufferHandle;

// Context

enum IndexType
{
	IT_UINT_16 = 0,
	IT_UINT_32 = 1
};

enum ImageFilter
{
	IF_NEAREST,
	IF_LINEAR,
	IF_CUBIC
};

enum ImageAspect
{
	IA_COLOR	= 0x00000001,
	IA_DEPTH	= 0x00000002,
	IA_STENCIL	= 0x00000004
};

enum RenderCommandContents
{
	RENDER_COMMANDS_INLINE = 0,
	RENDER_COMMANDS_SECONDARY_COMMAND_BUFFERS = 1
};

struct BlitRegion;
struct BufferImageCopy;
struct ImageSubresource;
struct RenderContextProcs;
struct RenderContext;
typedef void * CommandBufferHandle;

// Device

struct RenderDevice;
struct RenderDeviceInfo;
struct RenderDeviceProcs;

// Framebuffer

struct Framebuffer;
struct FramebufferDesc;
struct FramebufferAttachmentDesc;

// Memory

enum GPUMemoryType
{
	MT_GPU_LOCAL,
	MT_CPU_READ,
	MT_CPU_WRITE,
	MT_CPU_COHERENT
};

// Model

struct Mesh;
struct Model;
struct Vertex;
struct ModelRender;
struct ModelCreateInfo;

// Pipeline

enum BlendFactor
{
	RE_BF_ZERO						=  0,
	RE_BF_ONE						=  1,
	RE_BF_SRC_COLOR					=  2,
	RE_BF_ONE_MINUS_SRC_COLOR		=  3,
	RE_BF_DST_COLOR					=  4,
	RE_BF_ONE_MINUS_DST_COLOR		=  5,
	RE_BF_SRC_ALPHA					=  6,
	RE_BF_ONE_MINUS_SRC_ALPHA		=  7,
	RE_BF_DST_ALPHA					=  8,
	RE_BF_ONE_MINUS_DST_ALPHA		=  9,
	RE_BF_CONSTANT_COLOR			= 10,
	RE_BF_ONE_MINUS_CONSTANT_COLOR	= 11,
	RE_BF_CONSTANT_ALPHA			= 12,
	RE_BF_ONE_MINUS_CONSTANT_ALPHA	= 13,
	RE_BF_SRC_ALPHA_SATURATE		= 14,
	RE_BF_SRC1_COLOR				= 15,
	RE_BF_ONE_MINUS_SRC1_COLOR		= 16,
	RE_BF_SRC1_ALPHA				= 17,
	RE_BF_ONE_MINUS_SRC1_ALPHA		= 18,
};

enum BlendOperation
{
	RE_BOP_ADD					= 0,
	RE_BOP_SUBTRACT				= 1,
	RE_BOP_REVERSE_SUBTRACT		= 2,
	RE_BOP_MIN					= 3,
	RE_BOP_MAX					= 4
};
// TODO: See VK_EXT_blend_operation_advanced

struct BlendAttachmentDesc;
struct GraphicsPipelineDesc;
struct ComputePipelineDesc;

// Render Pass

enum AttachmentLoadOp
{
	ATL_LOAD = 0,
	ATL_CLEAR = 1,
	ATL_DONT_CARE = 2
};

enum AttachmentStoreOp
{
	ATS_STORE = 0,
	ATS_DONT_CARE = 1
};

enum AttachmentSampleCount
{
	ASC_1_SAMPLE		=  1,
	ASC_2_SAMPLES		=  2,
	ASC_4_SAMPLES		=  4,
	ASC_8_SAMPLES		=  8,
	ASC_16_SAMPLES		= 16
};

struct AttachmentDesc;
struct RenderPass;
struct RenderPassDesc;

// Sampler

enum SamplerMipmapMode
{
	SMM_NEAREST,
	SMM_LINEAR
};

enum SamplerAddressMode
{
	SAM_REPEAT,
	SAM_MIRRORED_REPEAT,
	SAM_CLAMP_TO_EDGE,
	SAM_CLAMP_TO_BORDER,
	SAM_MIRROR_CLAMP_TO_EDGE
};

enum CompareOperation
{
	CO_NEVER,
	CO_LESS,
	CO_EQUAL,
	CO_LESS_EQUAL,
	CO_GREATER,
	CO_NOT_EQUAL,
	CO_GREATER_EQUAL,
	CO_ALWAYS
};

enum BorderColor
{
	BC_FLOAT_TRANSPARENT_BLACK,
	BC_INT_TRANSPARENT_BLACK,
	BC_FLOAT_OPAQUE_BLACK,
	BC_INT_OPAQUE_BLACK,
	BC_FLOAT_OPAQUE_WHITE,
	BC_INT_OPAQUE_WHITE
};

struct Sampler;
struct SamplerDesc;

// Shader

enum ShaderType
{
	ST_GRAPHICS,
	ST_MESH,
	ST_COMPUTE,
	ST_RAY_TRACING
};

enum ShaderStage
{
	SS_VERTEX		= 0x00000001,
	SS_TESS_CTRL	= 0x00000002,
	SS_TESS_EVAL	= 0x00000004,
	SS_GEOMETRY		= 0x00000008,
	SS_FRAGMENT		= 0x00000010,
	SS_COMPUTE		= 0x00000020,
	SS_ALL_GRAPHICS	= 0x0000001F,
	SS_RAYGEN		= 0x00000100,
	SS_ANY_HIT		= 0x00000200,
	SS_CLOSEST_HIT	= 0x00000400,
	SS_MISS			= 0x00000800,
	SS_INTERSECTION = 0x00001000,
	SS_CALLABLE		= 0x00002000,
	SS_TASK			= 0x00000040,
	SS_MESH			= 0x00000080,
    SS_ALL			= 0x7FFFFFFF
};

struct Shader;
struct ShaderStageDesc;

// Shader Binding Table

enum ShaderEntryType
{
	SET_RayGen,
	SET_Miss,
	SET_HitGroup,
	SET_Callable
};

struct ShaderBindingTable;

// Swapchain

struct Surface;
struct Swapchain;

// Texture

enum TextureType
{
	TT_2D,
	TT_3D,
	TT_Cube,
	TT_2D_Multisample
};

enum TextureFormat
{
	TF_R8G8B8A8_UNORM,
	TF_R8G8B8A8_SRGB,
	TF_B8G8R8A8_UNORM,
	TF_B8G8R8A8_SRGB,
	TF_R16G16B16A16_SFLOAT,
	TF_R32G32B32A32_SFLOAT,

	TF_D32_SFLOAT,
	TF_D24_STENCIL8,

	TF_A2R10G10B10_UNORM,

	TF_R8G8_UNORM,

	TF_R8_UNORM,

	TF_BC5_UNORM,
	TF_BC5_SNORM,
	TF_BC6H_UF16,
	TF_BC6H_SF16,
	TF_BC7_UNORM,
	TF_BC7_SRGB,

	TF_ETC2_R8G8B8_UNORM,
	TF_ETC2_R8G8B8_SRGB,
	TF_ETC2_R8G8B8A1_UNORM,
	TF_ETC2_R8G8B8A1_SRGB,
	TF_EAC_R11_UNORM,
	TF_EAC_R11_SNORM,
	TF_EAC_R11G11_UNORM,
	TF_EAC_R11G11_SNORM,

	TF_INVALID
};

enum TextureUsage
{
	TU_TRANSFER_SRC					= 0x00000001,
	TU_TRANSFER_DST					= 0x00000002,
	TU_SAMPLED						= 0x00000004,
	TU_STORAGE						= 0x00000008,
	TU_COLOR_ATTACHMENT				= 0x00000010,
	TU_DEPTH_STENCIL_ATTACHMENT		= 0x00000020,
	TU_INPUT_ATTACHMENT				= 0x00000080,
	TU_SHADING_RATE_ATTACHMENT		= 0x00000100,
	TU_FRAGMENT_DENSITY_MAP			= 0x00000200
};

enum TextureLayout
{
	TL_UNKNOWN = 0,
	TL_COLOR_ATTACHMENT,
	TL_DEPTH_STENCIL_ATTACHMENT,
	TL_DEPTH_STENCIL_READ_ONLY_ATTACHMENT,
	TL_DEPTH_ATTACHMENT,
	TL_STENCIL_ATTACHMENT,
	TL_DEPTH_READ_ONLY_ATTACHMENT,
	TL_TRANSFER_SRC,
	TL_TRANSFER_DST,
	TL_SHADER_READ_ONLY,
	TL_PRESENT_SRC
};

struct Texture;
struct TextureDesc;
struct TextureResource;
struct TextureCreateInfo;
typedef uint16_t TextureHandle;

// Graph
struct RenderGraph;
struct RenderPass;
struct GraphResource;

// Synchronization
struct Fence;
struct Semaphore;

#endif /* _NE_RENDER_TYPES_H_ */
