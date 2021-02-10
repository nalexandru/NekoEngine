#ifndef _RE_TEXTURE_H_
#define _RE_TEXTURE_H_

#include <Engine/Types.h>
#include <Render/Device.h>

// one large texture set

enum TextureType
{
	TT_2D,
	TT_3D,
	TT_Cube
};

enum TextureFormat
{
	TF_R8G8B8A8_UNORM,
	TF_R8G8B8A8_UNORM_SRGB,
	TF_B8G8R8A8_UNORM,
	TF_B8G8R8A8_UNORM_SRGB,
	TF_R16G16B16A16_SFLOAT,
	TF_R32G32B32A32_SFLOAT,
	
	TF_R10G10B10A2_UNORM,
	
	TF_R8G8_UNORM,
	
	TF_R8_UNORM,
	
	TF_BC5_UNORM,
	TF_BC5_SNORM,
	TF_BC6H_UF16,
	TF_BC6H_SF16,
	TF_BC7_UNORM,
	TF_BC7_UNORM_SRGB,
	
	TF_COUNT
};

struct TextureDesc
{
	uint32_t width, height, depth;
	enum TextureType type;
	enum TextureFormat format;
	uint32_t arrayLayers, mipLevels;
};

struct TextureCreateInfo
{
	struct TextureDesc desc;
	void *data;
	uint64_t dataSize;
	bool keepData;
};

static inline struct Texture *Re_CreateTexture(struct RenderDevice *dev, const struct TextureCreateInfo *tci) { return Re_DeviceProcs.CreateTexture(dev, tci); };
static inline void Re_DestroyTexture(struct RenderDevice *dev, struct Texture *tex) { Re_DeviceProcs.DestroyTexture(dev, tex); }

//static inline void Re_UpdateTexture(struct Texture *tex, uint64_t offset, uint64_t size, void *data);

static inline const struct TextureDesc *Re_TextureDesc(const struct Texture *tex) { return NULL; }

#endif /* _RE_TEXTURE_H_ */
