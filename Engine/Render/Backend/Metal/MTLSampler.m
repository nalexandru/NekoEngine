#include "MTLBackend.h"

struct NeSampler *
Re_CreateSampler(const struct NeSamplerDesc *sDesc)
{
	MTLSamplerDescriptor *desc = [[MTLSamplerDescriptor alloc] init];
	
	desc.minFilter = NeToMTLTextureFilter(sDesc->minFilter);
	desc.magFilter = NeToMTLTextureFilter(sDesc->magFilter);
	desc.mipFilter = NeToMTLMipFilter(sDesc->mipmapMode);
	desc.sAddressMode = NeToMTLSamplerAddressMode(sDesc->addressModeU);
	desc.tAddressMode = NeToMTLSamplerAddressMode(sDesc->addressModeV);
	desc.rAddressMode = NeToMTLSamplerAddressMode(sDesc->addressModeW);
	desc.maxAnisotropy = sDesc->enableAnisotropy ? sDesc->maxAnisotropy : 0.f;
	desc.lodMinClamp = sDesc->minLod;
	desc.lodMaxClamp = sDesc->maxLod;
	desc.normalizedCoordinates = !sDesc->unnormalizedCoordinates;
	desc.compareFunction = NeToMTLSamplerCompareFunction(sDesc->compareOp);
	desc.borderColor = NeToMTLSamplerBorderColor(sDesc->borderColor);
	desc.supportArgumentBuffers = true;
	
	id<MTLSamplerState> s = [MTL_device newSamplerStateWithDescriptor: desc];

	[desc release];
	
	MTL_SetSampler(0, s);
	
	return (struct NeSampler *)s;
}

void
Re_DestroySampler(struct NeSampler *s)
{
	[(id<MTLSamplerState>)s release];
}
