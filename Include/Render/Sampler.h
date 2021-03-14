#ifndef _RE_SAMPLER_H_
#define _RE_SAMPLER_H_

#include <Engine/Types.h>
#include <Render/Device.h>

enum TextureFilter
{
	TFL_NEAREST,
	TFL_LINEAR
};

enum TextureMipmapMode
{
	TMM_NEAREST,
	TMM_LINEAR
};

enum TextureAddressMode
{
	TAM_REPEAT,
	TAM_MIRRORED_REPEAT,
	TAM_CLAMP_TO_EDGE,
	TAM_CLAMP_TO_BORDER,
	TAM_MIRROR_CLAMP_TO_EDGE
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

struct SamplerDesc
{
	enum TextureFilter minFilter, magFilter;
	enum TextureMipmapMode mipmapMode;
	enum TextureAddressMode addressModeU, addressModeV, addressModeW;
	bool enableAnisotropy;
	float maxAnisotropy;
	float minLod, maxLod, lodBias;
	bool unnormalizedCoordinates;
	bool enableCompare;
	enum CompareOperation compareOp;
	uint32_t borderColor;
};

struct Sampler;

static inline struct Sampler *Re_CreateSampler(const struct SamplerDesc *desc) { return Re_deviceProcs.CreateSampler(Re_device, desc); }
static inline void Re_DestroySampler(struct Sampler *s) { Re_deviceProcs.DestroySampler(Re_device, s); }

#endif /* _RE_SAMPLER_H_ */
