#ifndef _RE_TEXTURE_H_
#define _RE_TEXTURE_H_

#include <Engine/Types.h>
#include <Render/Device.h>
#include <Render/Memory.h>

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

static inline struct Texture *Re_CreateTexture(struct RenderDevice *dev, const struct TextureCreateInfo *tci) { return Re_deviceProcs.CreateTexture(dev, tci); };
static inline const struct TextureDesc *Re_TextureDesc(const struct Texture *tex) { return Re_deviceProcs.TextureDesc(tex); }
static inline void Re_DestroyTexture(struct RenderDevice *dev, struct Texture *tex) { Re_deviceProcs.DestroyTexture(dev, tex); }

//static inline void Re_UpdateTexture(struct Texture *tex, uint64_t offset, uint64_t size, void *data);

#endif /* _RE_TEXTURE_H_ */
