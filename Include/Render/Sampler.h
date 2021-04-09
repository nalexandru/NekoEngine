#ifndef _RE_SAMPLER_H_
#define _RE_SAMPLER_H_

#include <Render/Types.h>
#include <Render/Device.h>

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

#endif /* _RE_SAMPLER_H_ */
