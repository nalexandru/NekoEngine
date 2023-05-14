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

/* NekoEngine
 *
 * MTLSampler.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
