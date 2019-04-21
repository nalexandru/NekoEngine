/* NekoEngine
 *
 * dds.c
 * Author: Alexandru Naiman
 *
 * NekoEngine DDS Loader
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

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <graphics/texture.h>

 // DDS info:
 // https://msdn.microsoft.com/en-us/library/windows/desktop/bb943991(v=vs.85).aspx
 // https://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx
 // https://msdn.microsoft.com/en-us/library/windows/desktop/bb943984(v=vs.85).aspx
 // https://msdn.microsoft.com/en-us/library/windows/desktop/bb943983(v=vs.85).aspx
 // https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
 // https://msdn.microsoft.com/en-us/library/windows/desktop/bb172411(v=vs.85).aspx

#define DDS_MAGIC			0x20534444

 // surface flags
#define DDSD_CAPS			0x1
#define DDSD_HEIGHT			0x2
#define DDSD_WIDTH			0x4
#define DDSD_PITCH			0x8
#define DDSD_PIXELFORMAT		0x1000
#define DDSD_MIPMAPCOUNT		0x20000
#define DDSD_LINEARSIZE			0x80000
#define DDSD_DEPTH			0x800000

 // pixel format flags
#define DDPF_ALPHAPIXELS		0x1
#define DDPF_ALPHA			0x2
#define DDPF_FOURCC			0x4
#define DDPF_RGB			0x40
#define DDPF_RGBA			0x41
#define DDPF_YUV			0x200
#define DDPF_LUMINANCE			0x20000

 // dwCaps
#define DDSCAPS_COMPLEX			0x8
#define DDSCAPS_MIPMAP			0x400000
#define DDSCAPS_TEXTURE			0x1000

 // dwCaps2
#define DDSCAPS2_CUBEMAP		0x200
#define DDSCAPS2_CUBEMAP_POSITTIVEX	0x400
#define DDSCAPS2_CUBEMAP_NEGATIVEX	0x800
#define DDSCAPS2_CUBEMAP_POSITTIVEY	0x1000
#define DDSCAPS2_CUBEMAP_NEGATIVEY	0x2000
#define DDSCAPS2_CUBEMAP_POSITTIVEZ	0x8000
#define DDSCAPS2_CUBEMAP_ALLFACES	0xFC00
#define DDSCAPS2_VOLUME			0x200000

 // compressed texture types
#define FOURCC_DXT1 			0x31545844L // MAKEFOURCC('D','X','T','1')
#define FOURCC_DXT2 			0x32545844L // MAKEFOURCC('D','X','T','2')
#define FOURCC_DXT3 			0x33545844L // MAKEFOURCC('D','X','T','3')
#define FOURCC_DXT4 			0x34545844L // MAKEFOURCC('D','X','T','4')
#define FOURCC_DXT5 			0x35545844L // MAKEFOURCC('D','X','T','5')
#define FOURCC_DX10 			0x30315844L // MAKEFOURCC('D','X','1','0')

typedef enum DXGI_FORMAT {
	DXGI_FORMAT_UNKNOWN = 0,
	DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
	DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
	DXGI_FORMAT_R32G32B32A32_UINT = 3,
	DXGI_FORMAT_R32G32B32A32_SINT = 4,
	DXGI_FORMAT_R32G32B32_TYPELESS = 5,
	DXGI_FORMAT_R32G32B32_FLOAT = 6,
	DXGI_FORMAT_R32G32B32_UINT = 7,
	DXGI_FORMAT_R32G32B32_SINT = 8,
	DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
	DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
	DXGI_FORMAT_R16G16B16A16_UNORM = 11,
	DXGI_FORMAT_R16G16B16A16_UINT = 12,
	DXGI_FORMAT_R16G16B16A16_SNORM = 13,
	DXGI_FORMAT_R16G16B16A16_SINT = 14,
	DXGI_FORMAT_R32G32_TYPELESS = 15,
	DXGI_FORMAT_R32G32_FLOAT = 16,
	DXGI_FORMAT_R32G32_UINT = 17,
	DXGI_FORMAT_R32G32_SINT = 18,
	DXGI_FORMAT_R32G8X24_TYPELESS = 19,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
	DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
	DXGI_FORMAT_R10G10B10A2_UNORM = 24,
	DXGI_FORMAT_R10G10B10A2_UINT = 25,
	DXGI_FORMAT_R11G11B10_FLOAT = 26,
	DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
	DXGI_FORMAT_R8G8B8A8_UNORM = 28,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	DXGI_FORMAT_R8G8B8A8_UINT = 30,
	DXGI_FORMAT_R8G8B8A8_SNORM = 31,
	DXGI_FORMAT_R8G8B8A8_SINT = 32,
	DXGI_FORMAT_R16G16_TYPELESS = 33,
	DXGI_FORMAT_R16G16_FLOAT = 34,
	DXGI_FORMAT_R16G16_UNORM = 35,
	DXGI_FORMAT_R16G16_UINT = 36,
	DXGI_FORMAT_R16G16_SNORM = 37,
	DXGI_FORMAT_R16G16_SINT = 38,
	DXGI_FORMAT_R32_TYPELESS = 39,
	DXGI_FORMAT_D32_FLOAT = 40,
	DXGI_FORMAT_R32_FLOAT = 41,
	DXGI_FORMAT_R32_UINT = 42,
	DXGI_FORMAT_R32_SINT = 43,
	DXGI_FORMAT_R24G8_TYPELESS = 44,
	DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
	DXGI_FORMAT_R8G8_TYPELESS = 48,
	DXGI_FORMAT_R8G8_UNORM = 49,
	DXGI_FORMAT_R8G8_UINT = 50,
	DXGI_FORMAT_R8G8_SNORM = 51,
	DXGI_FORMAT_R8G8_SINT = 52,
	DXGI_FORMAT_R16_TYPELESS = 53,
	DXGI_FORMAT_R16_FLOAT = 54,
	DXGI_FORMAT_D16_UNORM = 55,
	DXGI_FORMAT_R16_UNORM = 56,
	DXGI_FORMAT_R16_UINT = 57,
	DXGI_FORMAT_R16_SNORM = 58,
	DXGI_FORMAT_R16_SINT = 59,
	DXGI_FORMAT_R8_TYPELESS = 60,
	DXGI_FORMAT_R8_UNORM = 61,
	DXGI_FORMAT_R8_UINT = 62,
	DXGI_FORMAT_R8_SNORM = 63,
	DXGI_FORMAT_R8_SINT = 64,
	DXGI_FORMAT_A8_UNORM = 65,
	DXGI_FORMAT_R1_UNORM = 66,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
	DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
	DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
	DXGI_FORMAT_BC1_TYPELESS = 70,
	DXGI_FORMAT_BC1_UNORM = 71,
	DXGI_FORMAT_BC1_UNORM_SRGB = 72,
	DXGI_FORMAT_BC2_TYPELESS = 73,
	DXGI_FORMAT_BC2_UNORM = 74,
	DXGI_FORMAT_BC2_UNORM_SRGB = 75,
	DXGI_FORMAT_BC3_TYPELESS = 76,
	DXGI_FORMAT_BC3_UNORM = 77,
	DXGI_FORMAT_BC3_UNORM_SRGB = 78,
	DXGI_FORMAT_BC4_TYPELESS = 79,
	DXGI_FORMAT_BC4_UNORM = 80,
	DXGI_FORMAT_BC4_SNORM = 81,
	DXGI_FORMAT_BC5_TYPELESS = 82,
	DXGI_FORMAT_BC5_UNORM = 83,
	DXGI_FORMAT_BC5_SNORM = 84,
	DXGI_FORMAT_B5G6R5_UNORM = 85,
	DXGI_FORMAT_B5G5R5A1_UNORM = 86,
	DXGI_FORMAT_B8G8R8A8_UNORM = 87,
	DXGI_FORMAT_B8G8R8X8_UNORM = 88,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	DXGI_FORMAT_BC6H_TYPELESS = 94,
	DXGI_FORMAT_BC6H_UF16 = 95,
	DXGI_FORMAT_BC6H_SF16 = 96,
	DXGI_FORMAT_BC7_TYPELESS = 97,
	DXGI_FORMAT_BC7_UNORM = 98,
	DXGI_FORMAT_BC7_UNORM_SRGB = 99,
	DXGI_FORMAT_AYUV = 100,
	DXGI_FORMAT_Y410 = 101,
	DXGI_FORMAT_Y416 = 102,
	DXGI_FORMAT_NV12 = 103,
	DXGI_FORMAT_P010 = 104,
	DXGI_FORMAT_P016 = 105,
	DXGI_FORMAT_420_OPAQUE = 106,
	DXGI_FORMAT_YUY2 = 107,
	DXGI_FORMAT_Y210 = 108,
	DXGI_FORMAT_Y216 = 109,
	DXGI_FORMAT_NV11 = 110,
	DXGI_FORMAT_AI44 = 111,
	DXGI_FORMAT_IA44 = 112,
	DXGI_FORMAT_P8 = 113,
	DXGI_FORMAT_A8P8 = 114,
	DXGI_FORMAT_B4G4R4A4_UNORM = 115,
	DXGI_FORMAT_P208 = 130,
	DXGI_FORMAT_V208 = 131,
	DXGI_FORMAT_V408 = 132,
//	DXGI_FORMAT_FORCE_UINT = 0xffffffff
} DXGI_FORMAT;

typedef enum D3D10_RESOURCE_DIMENSION {
	D3D10_RESOURCE_DIMENSION_UNKNOWN = 0,
	D3D10_RESOURCE_DIMENSION_BUFFER = 1,
	D3D10_RESOURCE_DIMENSION_TEXTURE1D = 2,
	D3D10_RESOURCE_DIMENSION_TEXTURE2D = 3,
	D3D10_RESOURCE_DIMENSION_TEXTURE3D = 4
} D3D10_RESOURCE_DIMENSION;

typedef struct
{
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwFourCC;
	uint32_t dwRGBBitCount;
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwABitMask;
} DDS_PIXELFORMAT;

typedef struct
{
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwHeight;
	uint32_t dwWidth;
	uint32_t dwPitchOrLinearSize;
	uint32_t dwDepth;
	uint32_t dwMipMapCount;
	uint32_t dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t dwCaps;
	uint32_t dwCaps2;
	uint32_t dwCaps3;
	uint32_t dwCaps4;
	uint32_t dwReserved2;
} DDS_HEADER;

typedef struct
{
	uint32_t dxgiFormat;
	D3D10_RESOURCE_DIMENSION resourceDimension;
	uint32_t miscFlag;
	uint32_t arraySize;
	uint32_t miscFlags2;
} DDS_HEADER_DXT10;

ne_status
asset_load_dds(
	const uint8_t *data,
	uint64_t data_size,
	uint32_t *width,
	uint32_t *height,
	uint32_t *depth,
	ne_texture_type *type,
	ne_image_format *format,
	uint32_t *levels,
	uint32_t *layers,
	uint8_t **img_data,
	uint64_t *img_data_size,
	bool *should_free)
{
	if (!data || !data_size || !width ||
		!height || !depth || !format ||
		!levels || !img_data || !img_data_size ||
		!type || !layers || !should_free)
		return NE_INVALID_ARGS;

	DDS_HEADER hdr;
	DDS_HEADER_DXT10 hdr_DXT10;
	uint64_t offset = 0;
	uint32_t magic = 0;
	uint32_t block_size = 0;
	bool DXT10 = false;

	memcpy(&magic, data + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	if (magic != DDS_MAGIC)
		return NE_INVALID_HEADER;

	memcpy(&hdr, data + offset, sizeof(hdr));
	offset += sizeof(hdr);

	memcpy(&hdr_DXT10, data + offset, sizeof(hdr_DXT10));
	offset += sizeof(hdr_DXT10);

	*img_data_size = data_size - offset;

	*type = NE_TEXTURE_2D;
	block_size = 16;

	if (hdr.dwSize != sizeof(hdr))
		return NE_INVALID_HEADER;

	*format = NE_IMAGE_FORMAT_UNKNOWN;

	if (hdr.ddspf.dwFlags & DDPF_FOURCC) {
		switch (hdr.ddspf.dwFourCC) {
		case FOURCC_DXT1: {
			*format = NE_IMAGE_FORMAT_BC1_RGBA_UNORM_BLOCK;
			block_size = 8;
		} break;
		case FOURCC_DXT2: {
			*format = NE_IMAGE_FORMAT_BC2_UNORM_BLOCK;
		} break;
		case FOURCC_DXT3: {
			*format = NE_IMAGE_FORMAT_BC3_UNORM_BLOCK;
		} break;
		case FOURCC_DXT4: {
			*format = NE_IMAGE_FORMAT_BC4_UNORM_BLOCK;
			block_size = 8;
		} break;
		case FOURCC_DXT5: {
			*format = NE_IMAGE_FORMAT_BC5_UNORM_BLOCK;
		} break;
		case FOURCC_DX10: {
			switch (hdr_DXT10.dxgiFormat) {
			case DXGI_FORMAT_BC1_UNORM:
				*format = NE_IMAGE_FORMAT_BC1_RGBA_UNORM_BLOCK;
				block_size = 8;
			break;
			case DXGI_FORMAT_BC1_UNORM_SRGB:
				*format = NE_IMAGE_FORMAT_BC1_RGBA_SRGB_BLOCK;
				block_size = 8;
			break;
			case DXGI_FORMAT_BC2_UNORM:
				*format = NE_IMAGE_FORMAT_BC2_UNORM_BLOCK;
			break;
			case DXGI_FORMAT_BC2_UNORM_SRGB:
				*format = NE_IMAGE_FORMAT_BC2_SRGB_BLOCK;
			break;
			case DXGI_FORMAT_BC3_UNORM:
				*format = NE_IMAGE_FORMAT_BC3_UNORM_BLOCK;
			break;
			case DXGI_FORMAT_BC3_UNORM_SRGB:
				*format = NE_IMAGE_FORMAT_BC3_SRGB_BLOCK;
			break;
			case DXGI_FORMAT_BC4_UNORM:
				*format = NE_IMAGE_FORMAT_BC4_UNORM_BLOCK;
				block_size = 8;
			break;
			case DXGI_FORMAT_BC4_SNORM:
				*format = NE_IMAGE_FORMAT_BC4_SNORM_BLOCK;
				block_size = 8;
			break;
			case DXGI_FORMAT_BC5_UNORM:
				*format = NE_IMAGE_FORMAT_BC5_UNORM_BLOCK;
			break;
			case DXGI_FORMAT_BC5_SNORM:
				*format = NE_IMAGE_FORMAT_BC5_SNORM_BLOCK;
			break;
			case DXGI_FORMAT_BC6H_UF16:
				*format = NE_IMAGE_FORMAT_BC6H_UFLOAT_BLOCK;
			break;
			case DXGI_FORMAT_BC6H_SF16:
				*format = NE_IMAGE_FORMAT_BC6H_SFLOAT_BLOCK;
			break;
			case DXGI_FORMAT_BC7_UNORM:
				*format = NE_IMAGE_FORMAT_BC7_UNORM_BLOCK;
			break;
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				*format = NE_IMAGE_FORMAT_BC7_SRGB_BLOCK;
			break;
			}

			switch (hdr_DXT10.resourceDimension) {
			case D3D10_RESOURCE_DIMENSION_TEXTURE1D:
				*type = NE_TEXTURE_1D;
			break;
			case D3D10_RESOURCE_DIMENSION_TEXTURE2D:
				*type = NE_TEXTURE_2D;
			break;
			case D3D10_RESOURCE_DIMENSION_TEXTURE3D:
				*type = NE_TEXTURE_3D;
			break;
			}

			DXT10 = true;
		}
		}
	}

	if (*format == NE_IMAGE_FORMAT_UNKNOWN)
		return NE_INVALID_HEADER;

	if (!DXT10) {
		offset -= sizeof(DDS_HEADER_DXT10);
		*img_data_size += sizeof(DDS_HEADER_DXT10);
	}

	*img_data = (uint8_t *)data + offset;
	*width = hdr.dwWidth;
	*height = hdr.dwHeight;
	*depth = !hdr.dwDepth ? 1 : hdr.dwDepth;
	*levels = 1;//hdr.dwMipMapCount; FIXME
	*layers = 1;
	*should_free = false;

	if (hdr.dwCaps2 & DDSCAPS2_CUBEMAP) {
		*type = NE_TEXTURE_CUBEMAP;
		*layers = 6;
	}

	return NE_OK;
}
