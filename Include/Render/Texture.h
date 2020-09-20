#ifndef _RE_TEXTURE_H_
#define _RE_TEXTURE_H_

#include <Engine/Types.h>

#define RES_TEXTURE "Texture"

#ifdef __cplusplus
extern "C" {
#endif

enum TextureType
{
	TT_2D,
	TT_3D,
	TT_Cube
};

enum TextureFormat
{
	TF_R8G8B8A8_UNORM = 0,
	TF_R8G8B8A8_UNORM_SRGB,
	TF_B8G8R8A8_UNORM,
	TF_B8G8R8A8_UNORM_SRGB,
	TF_B8G8R8X8_UNORM,
	TF_B8G8R8X8_UNORM_SRGB,
	TF_R16G16B16A16_SFLOAT,
	TF_R16G16B16A16_UNORM,
	TF_R32G32B32A32_SFLOAT,
	TF_R32G32B32A32_UINT,
	TF_R10G10B10A2_UNORM,
	TF_R32G32B32_SFLOAT,
	TF_R32G32B32_UINT,
	TF_R8G8_UNORM,
	TF_R16G16_SFLOAT,
	TF_R16G16_UNORM,
	TF_R32G32_SFLOAT,
	TF_R32G32_UINT,
	TF_R8_UNORM,
	TF_R16_SFLOAT,
	TF_R16_UNORM,
	TF_R32_SFLOAT,
	TF_R32_UINT,
	TF_BC1_UNORM,
	TF_BC1_UNORM_SRGB,
	TF_BC2_UNORM,
	TF_BC2_UNORM_SRGB,
	TF_BC3_UNORM,
	TF_BC3_UNORM_SRGB,
	TF_BC4_UNORM,
	TF_BC4_SNORM,
	TF_BC5_UNORM,
	TF_BC5_SNORM,
	TF_BC6H_UF16,
	TF_BC6H_SF16,
	TF_BC7_UNORM,
	TF_BC7_UNORM_SRGB
};

struct Texture
{
	uint16_t width, height, depth, levels;
	enum TextureType type;
	enum TextureFormat format;
	void *data;
	uint32_t dataSize;
	uint8_t renderDataStart;
};

struct TextureCreateInfo
{
	uint16_t width, height, depth;
	enum TextureType type;
	enum TextureFormat format;
	void *data;
	uint32_t dataSize;
	bool keepData;
};

// shared resource handling
bool Re_CreateTexture(const char *name, const struct TextureCreateInfo *ci, struct Texture *tex, Handle h);
bool Re_LoadTexture(struct ResourceLoadInfo *li, const char *args, struct Texture *tex, Handle h);
void Re_UnloadTexture(struct Texture *tex, Handle h);

// Implemented in render library
bool Re_InitTexture(const char *name, struct Texture *tex, Handle h);
bool Re_UpdateTexture(struct Texture *tex, const void *data, uint64_t offset, uint64_t size);
void Re_TermTexture(struct Texture *tex);

#ifdef __cplusplus
}
#endif

#endif /* _RE_TEXTURE_H_ */