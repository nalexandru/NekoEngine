/* NekoEngine
 *
 * texture.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Texture
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _NE_GRAPHICS_TEXTURE_H_
#define _NE_GRAPHICS_TEXTURE_H_

#include <stdint.h>
#include <stdbool.h>

#include <engine/status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ne_texture_type
{
	NE_TEXTURE_1D = 0,
	NE_TEXTURE_2D,
	NE_TEXTURE_3D,
	NE_TEXTURE_CUBEMAP,
	NE_TEXTURE_TYPE_COUNT
} ne_texture_type;

typedef enum ne_image_format
{
	NE_IMAGE_FORMAT_UNKNOWN = 0,
	NE_IMAGE_FORMAT_R8_UNORM,
	NE_IMAGE_FORMAT_R8_UINT,
	NE_IMAGE_FORMAT_R8G8_UNORM,
	NE_IMAGE_FORMAT_R8G8B8_UNORM,
	NE_IMAGE_FORMAT_R8G8B8_SRGB,
	NE_IMAGE_FORMAT_R8G8B8A8_UNORM,
	NE_IMAGE_FORMAT_R8G8B8A8_SRGB,
	NE_IMAGE_FORMAT_R16G16B16A16_UINT,
	NE_IMAGE_FORMAT_R16G16B16A16_UNORM,
	NE_IMAGE_FORMAT_R16G16B16A16_SFLOAT,
	NE_IMAGE_FORMAT_R32G32B32A32_UINT,
	NE_IMAGE_FORMAT_R32G32B32A32_SFLOAT,
	NE_IMAGE_FORMAT_R64G64B64A64_UINT,
	NE_IMAGE_FORMAT_R64G64B64A64_SFLOAT,
	NE_IMAGE_FORMAT_BC1_RGBA_UNORM_BLOCK,
	NE_IMAGE_FORMAT_BC1_RGBA_SRGB_BLOCK,
	NE_IMAGE_FORMAT_BC2_UNORM_BLOCK,
	NE_IMAGE_FORMAT_BC2_SRGB_BLOCK,
	NE_IMAGE_FORMAT_BC3_UNORM_BLOCK,
	NE_IMAGE_FORMAT_BC3_SRGB_BLOCK,
	NE_IMAGE_FORMAT_BC4_UNORM_BLOCK,
	NE_IMAGE_FORMAT_BC4_SNORM_BLOCK,
	NE_IMAGE_FORMAT_BC5_UNORM_BLOCK,
	NE_IMAGE_FORMAT_BC5_SNORM_BLOCK,
	NE_IMAGE_FORMAT_BC6H_UFLOAT_BLOCK,
	NE_IMAGE_FORMAT_BC6H_SFLOAT_BLOCK,
	NE_IMAGE_FORMAT_BC7_UNORM_BLOCK,
	NE_IMAGE_FORMAT_BC7_SRGB_BLOCK,

	NE_IMAGE_FORMAT_COUNT
} ne_image_format;

typedef enum ne_texture_format
{
	NE_TEXTURE_FORMAT_DDS,
	NE_TEXTURE_FORMAT_TGA,
	NE_TEXTURE_FORMAT_PNG
} ne_texture_format;

struct ne_texture_create_info
{
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t levels;
	uint32_t layers;
	ne_texture_type type;
	ne_image_format format;
};

struct ne_texture;

#define RES_TEXTURE	"res_texture"

#ifdef _NE_ENGINE_INTERNAL_

void	*load_texture(const char *path);

#endif /* _NE_ENGINE_INTERNAL */

#ifdef __cplusplus
}
#endif

#endif /* _NE_GRAPHICS_TEXTURE_H_ */

