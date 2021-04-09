#ifndef _RE_TEXTURE_H_
#define _RE_TEXTURE_H_

#include <Render/Types.h>
#include <Render/Device.h>

#define RES_TEXTURE "Texture"

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

#endif /* _RE_TEXTURE_H_ */
