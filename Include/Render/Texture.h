#ifndef _RE_TEXTURE_H_
#define _RE_TEXTURE_H_

#include <Engine/Types.h>
#include <Render/Device.h>
#include <Render/Memory.h>

#define RES_TEXTURE "Texture"

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
	TL_DEPTH_STENCIL_READ_ONLY,
	TL_TRANSFER_SRC,
	TL_TRANSFER_DST,
	TL_SHADER_READ_ONLY
};

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
};

bool Re_CreateTextureResource(const char *name, const struct TextureCreateInfo *ci, struct TextureResource *tex, Handle h);
bool Re_LoadTextureResource(struct ResourceLoadInfo *li, const char *args, struct TextureResource *tex, Handle h);
void Re_UnloadTextureResource(struct TextureResource *tex, Handle h);

static inline const struct TextureDesc *Re_TextureDesc(const struct Texture *tex) { return Re_deviceProcs.TextureDesc(tex); }
static inline enum TextureLayout Re_TextureLayout(const struct Texture *tex) { return Re_deviceProcs.TextureLayout(tex); }

//static inline void Re_UpdateTexture(struct Texture *tex, uint64_t offset, uint64_t size, void *data);

struct Sampler *Re_SceneTextureSampler(void);
const struct DescriptorSetLayout *Re_TextureDescriptorSetLayout(void);
void Re_BindTextureDescriptorSet(struct PipelineLayout *layout, uint32_t pos);

bool Re_InitTextureSystem(void);
void Re_TermTextureSystem(void);

#endif /* _RE_TEXTURE_H_ */
