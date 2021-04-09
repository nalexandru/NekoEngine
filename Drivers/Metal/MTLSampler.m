#define Handle __EngineHandle

#include <Render/Device.h>
#include <Render/Sampler.h>

#undef Handle

#include "MTLDriver.h"

id<MTLSamplerState>
MTL_CreateSampler(id<MTLDevice> dev, const struct SamplerDesc *sDesc)
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
	
	id<MTLSamplerState> s = [dev newSamplerStateWithDescriptor: desc];

	[desc autorelease];
	
	MTL_SetSampler(0, s);
	
	return s;
}

void
MTL_DestroySampler(id<MTLDevice> dev, id<MTLSamplerState> s)
{
	[s autorelease];
}
