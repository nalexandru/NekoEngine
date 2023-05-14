#include "D3D12Backend.h"

#include <Engine/Config.h>

uint32_t D3D12Bk_staticSamplerCount = 0;
D3D12_STATIC_SAMPLER_DESC D3D12Bk_staticSamplers[10]{};

static D3D12_STATIC_BORDER_COLOR
NeToD3D12BorderColor(enum NeBorderColor c)
{
	switch (c) {
		case BC_FLOAT_OPAQUE_BLACK:
		case BC_INT_OPAQUE_BLACK: return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		case BC_FLOAT_OPAQUE_WHITE:
		case BC_INT_OPAQUE_WHITE: return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		case BC_FLOAT_TRANSPARENT_BLACK:
		case BC_INT_TRANSPARENT_BLACK:
		default: return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	}
}

static D3D12_COMPARISON_FUNC
NeToD3D12CompareFunc(enum NeCompareOperation op)
{
	switch (op)
	{
	case CO_NEVER: return D3D12_COMPARISON_FUNC_NEVER;
	case CO_LESS: return D3D12_COMPARISON_FUNC_LESS;
	case CO_EQUAL: return D3D12_COMPARISON_FUNC_EQUAL;
	case CO_LESS_EQUAL: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case CO_GREATER: return D3D12_COMPARISON_FUNC_GREATER;
	case CO_NOT_EQUAL: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case CO_GREATER_EQUAL: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case CO_ALWAYS:
	default: return D3D12_COMPARISON_FUNC_ALWAYS;
	}
}

static inline D3D12_TEXTURE_ADDRESS_MODE
NeToD3D12AddressMode(enum NeSamplerAddressMode mode)
{
	switch (mode)
	{
	case SAM_REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case SAM_MIRRORED_REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case SAM_CLAMP_TO_BORDER: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	case SAM_MIRROR_CLAMP_TO_EDGE: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	case SAM_CLAMP_TO_EDGE:
	default:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	}
}

static inline D3D12_FILTER_TYPE
NeToD3D12MipmapMode(enum NeSamplerMipmapMode m)
{
	switch (m) {
	case SMM_NEAREST: return D3D12_FILTER_TYPE_POINT;
	case SMM_LINEAR:
	default: return D3D12_FILTER_TYPE_LINEAR;
	}
}

static inline D3D12_FILTER_TYPE
NeToD3D12FilterType(enum NeImageFilter f)
{
	switch (f) {
	case IF_NEAREST: return D3D12_FILTER_TYPE_POINT;
	case IF_LINEAR:
	default: return D3D12_FILTER_TYPE_LINEAR;
	}
}

struct NeSampler *
Re_CreateSampler(const struct NeSamplerDesc *desc)
{
	if (E_GetCVarBln("Render_AnisotropicFiltering", true)->bln) {
		D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(
			desc->enableCompare ? D3D12_FILTER_REDUCTION_TYPE_COMPARISON : D3D12_FILTER_REDUCTION_TYPE_STANDARD
		);
	} else {
		D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].Filter = D3D12_ENCODE_BASIC_FILTER(
			NeToD3D12FilterType(desc->minFilter), NeToD3D12FilterType(desc->magFilter),
			NeToD3D12MipmapMode(desc->mipmapMode),
			desc->enableCompare ? D3D12_FILTER_REDUCTION_TYPE_COMPARISON : D3D12_FILTER_REDUCTION_TYPE_STANDARD
		);
	}

	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].AddressU = NeToD3D12AddressMode(desc->addressModeU);
	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].AddressV = NeToD3D12AddressMode(desc->addressModeV);
	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].AddressW = NeToD3D12AddressMode(desc->addressModeW);
	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].MipLODBias = desc->lodBias;
	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].MaxAnisotropy = (UINT)desc->maxAnisotropy;
	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].BorderColor = NeToD3D12BorderColor((enum NeBorderColor)desc->borderColor);
	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].ComparisonFunc = NeToD3D12CompareFunc(desc->compareOp);
	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].MinLOD = desc->minLod;
	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].MaxLOD = desc->maxLod;
	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].RegisterSpace = 0;
	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	D3D12Bk_staticSamplers[D3D12Bk_staticSamplerCount].ShaderRegister = D3D12Bk_staticSamplerCount++;

	return (struct NeSampler *)(uintptr_t)D3D12Bk_staticSamplerCount;
}

void
Re_DestroySampler(struct NeSampler *s)
{
}

/* NekoEngine
 *
 * D3D12Sampler.cxx
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
