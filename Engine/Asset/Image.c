#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <System/Endian.h>
#include <Render/Render.h>
#include <System/Log.h>

#include <png.h>
#include <jpeglib.h>
#include <jerror.h>

#define STBI_NO_STDIO
#define STBI_MALLOC(sz)			Sys_Alloc(1, sz, MH_Asset)
#define STBI_REALLOC(p, new)	Sys_ReAlloc(p, 1, new, MH_Asset)
#define STBI_FREE(p)			Sys_Free(p)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define IMGMOD	"Asset_ImageLoader"

static int ImgRead(void *user, char *data, int size);
static void ImgSkip(void *user, int n);
static int ImgEof(void *user);

static void PNGRead(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead);

static stbi_io_callbacks f_cb = { ImgRead, ImgSkip, ImgEof };

static inline bool LoadSTBI(struct NeStream *stm, struct NeTextureCreateInfo *tci, int rcomp);

static uint8_t f_hdrHeader[11] = { 0x23, 0x3F, 0x52, 0x41, 0x44, 0x49, 0x41, 0x4E, 0x43, 0x45, 0x0A };

bool
Asset_LoadImageComp(struct NeStream *stm, struct NeTextureCreateInfo *tci, bool flip, int rcomp)
{
	uint8_t hdr[11];

	tci->desc.type = TT_2D;

	E_ReadStream(stm, hdr, sizeof(hdr));
	E_SeekStream(stm, -((int64_t)sizeof(hdr)), IO_SEEK_CUR);

	bool rc = false;
	if (png_check_sig(hdr, sizeof(hdr))) {
		E_SeekStream(stm, 8, IO_SEEK_CUR);
		rc = Asset_LoadPNG(stm, tci);
	} else if (hdr[0] == 0xFF && hdr[1] == 0xD8) {
		rc = Asset_LoadJPG(stm, tci);
	} else if (!memcmp(hdr, f_hdrHeader, sizeof(hdr))) {
		rc = LoadSTBI(stm, tci, rcomp);
	} else {
		rc = LoadSTBI(stm, tci, rcomp);
	}

	if (!rc || !flip)
		return rc;

	if (tci->desc.format == TF_R8G8B8A8_UNORM) {
		for (uint32_t i = 0; i < tci->desc.height / 2; ++i) {
			uint32_t *sptr = ((uint32_t *)tci->data) + tci->desc.width * i;
			uint32_t *dptr = ((uint32_t *)tci->data) + tci->desc.width * (tci->desc.height - i - 1);

			for (uint32_t j = 0; j < tci->desc.width; ++j) {
				*sptr = *sptr ^ *dptr;
				*dptr = *sptr ^ *dptr;
				*sptr = *sptr ^ *dptr++;
				++sptr;
			}
		}
	} else if (tci->desc.format == TF_R16G16B16A16_UNORM) {
		for (uint32_t i = 0; i < tci->desc.height / 2; ++i) {
			uint64_t *sptr = ((uint64_t *)tci->data) + tci->desc.width * i;
			uint64_t *dptr = ((uint64_t *)tci->data) + tci->desc.width * (tci->desc.height - i - 1);

			for (uint32_t j = 0; j < tci->desc.width; ++j) {
				*sptr = *sptr ^ *dptr;
				*dptr = *sptr ^ *dptr;
				*sptr = *sptr ^ *dptr++;
				++sptr;
			}
		}
	} else {

	}

	return true;
}

bool
Asset_LoadHDR(struct NeStream *stm, struct NeTextureCreateInfo *tci)
{
	int x, y, comp;
	float *image = stbi_loadf_from_callbacks(&f_cb, stm, &x, &y, &comp, 4);
	if (!image)
		return false;

	tci->desc.type = TT_2D;
	tci->desc.format = TF_R32G32B32A32_SFLOAT;
	tci->desc.width = (uint16_t)x;
	tci->desc.height = (uint16_t)y;
	tci->desc.mipLevels = tci->desc.arrayLayers = 1;
	tci->data = image;
	tci->dataSize = sizeof(*image) * 4 * x * y;

	return true;
}

bool
Asset_LoadPNG(struct NeStream *stm, struct NeTextureCreateInfo *tci)
{
	int32_t bitsPerChannel, colorType;

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);

	png_set_read_fn(png, stm, PNGRead);
	png_set_sig_bytes(png, 8);
	png_read_info(png, info);

	if (png_get_IHDR(png, info, &tci->desc.width, &tci->desc.height, &bitsPerChannel, &colorType, NULL, NULL, NULL) != 1) {
		Sys_LogEntry(IMGMOD, LOG_CRITICAL, "Failed to load PNG header");
		png_destroy_read_struct(&png, &info, NULL);
		return false;
	}

	int comp = 4;
	switch (colorType) {
	case PNG_COLOR_TYPE_GRAY: tci->desc.format = TF_R8_UNORM; comp = 1; break;
	case PNG_COLOR_TYPE_RGBA: tci->desc.format = TF_R8G8B8A8_UNORM; break;
	}

	tci->dataSize = sizeof(uint8_t) * comp * tci->desc.width * tci->desc.height;
	tci->data = Sys_Alloc(tci->dataSize, 1, MH_Asset);

	const size_t rowSize = png_get_rowbytes(png, info);
	if (rowSize == tci->desc.width * comp) {
		for (uint32_t i = 0; i < tci->desc.height; ++i)
			png_read_row(png, (uint8_t *)tci->data + rowSize * i, NULL);
	} else {
		Sys_LogEntry(IMGMOD, LOG_CRITICAL, "Failed to load PNG file: component number mismatch");
		png_destroy_read_struct(&png, &info, NULL);
		return false;
	}

	tci->desc.mipLevels = tci->desc.arrayLayers = 1;

	png_destroy_read_struct(&png, &info, NULL);
	return true;
}

bool
Asset_LoadJPG(struct NeStream *stm, struct NeTextureCreateInfo *tci)
{
	struct jpeg_decompress_struct info = {0};

	void *jpeg = E_ReadStreamBlob(stm, MH_Asset);

	struct jpeg_error_mgr err;
	info.err = jpeg_std_error(&err);

	jpeg_create_decompress(&info);
	jpeg_mem_src(&info, jpeg, (unsigned long) E_StreamLength(stm));

	jpeg_read_header(&info, true);
	jpeg_start_decompress(&info);

	if (info.data_precision == 12) {
		Sys_LogEntry("JPEG", LOG_CRITICAL, "12 bit JPEG files are not supported. Please use 8 or 16 bit files.");
		jpeg_finish_decompress(&info);
		jpeg_destroy_decompress(&info);
		Sys_Free(jpeg);
		return false;
	}

	union {
		J16SAMPARRAY s16;
		J12SAMPARRAY s12;
		JSAMPARRAY s;
	} buff;

	const int stride = info.output_width * info.output_components;
	buff.s = info.mem->alloc_sarray((j_common_ptr)&info, JPOOL_IMAGE, stride, 1);

	tci->desc.width = info.output_width;
	tci->desc.height = info.output_height;
	tci->desc.mipLevels = tci->desc.arrayLayers = 1;

	if (info.data_precision == 16) {
		tci->desc.format = TF_R16G16B16A16_UNORM;
		tci->dataSize = sizeof(uint16_t) * 4 * tci->desc.width * tci->desc.height;
		tci->data = Sys_Alloc(tci->dataSize, 1, MH_Asset);

		uint16_t *dptr = tci->data;
		while (info.output_scanline < info.output_height) {
			jpeg16_read_scanlines(&info, buff.s16, 1);

			uint16_t *sptr = buff.s16[0];
			for (uint32_t i = 0; i < info.output_width; ++i) {
				memcpy(dptr, sptr, 6); dptr += 3; sptr += 3;
				*dptr++ = 0xFFFF;
			}
		}
	} else {
		tci->desc.format = TF_R8G8B8A8_UNORM;
		tci->dataSize = sizeof(uint8_t) * 4 * tci->desc.width * tci->desc.height;
		tci->data = Sys_Alloc(tci->dataSize, 1, MH_Asset);

		uint8_t *dptr = tci->data;
		while (info.output_scanline < info.output_height) {
			jpeg_read_scanlines(&info, buff.s, 1);

			uint8_t *sptr = buff.s[0];
			if (info.out_color_components == 1) {
				for (uint32_t i = 0; i < info.output_width; ++i) {
					*dptr++ = *sptr;
					*dptr++ = *sptr;
					*dptr++ = *sptr++;
					*dptr++ = 0xFF;
				}
			} else if (info.out_color_components == 3) {
				for (uint32_t i = 0; i < info.output_width; ++i) {
					memcpy(dptr, sptr, 3); dptr += 3; sptr += 3;
					*dptr++ = 0xFF;
				}
			}
		}
	}

	jpeg_finish_decompress(&info);
	jpeg_destroy_decompress(&info);

	Sys_Free(jpeg);

	return true;
}

bool
Asset_LoadSTBI(struct NeStream *stm, struct NeTextureCreateInfo *tci)
{
	return LoadSTBI(stm, tci, 4);
}

bool
LoadSTBI(struct NeStream *stm, struct NeTextureCreateInfo *tci, int rcomp)
{
	int x, y, comp;

	stbi_uc *image = stbi_load_from_callbacks(&f_cb, stm, &x, &y, &comp, rcomp);
	if (!image)
		return false;

	tci->data = image;
	tci->dataSize = sizeof(*image) * rcomp * x * y;

	switch (comp) {
	case 1: tci->desc.format = TF_R8_UNORM; break;
	case 2: tci->desc.format = TF_R8G8_UNORM; break;
	case 4: tci->desc.format = TF_R8G8B8A8_UNORM; break;
	}

	tci->desc.width = x;
	tci->desc.height = y;
	tci->desc.mipLevels = tci->desc.arrayLayers = 1;

	return true;
}

int
ImgRead(void *user, char *data, int size)
{
	return (int)E_ReadStream((struct NeStream *)user, data, size);
}

void
ImgSkip(void *user, int n)
{
	E_SeekStream((struct NeStream *) user, n, IO_SEEK_CUR);
}

int
ImgEof(void *user)
{
	return E_EndOfStream((struct NeStream *)user);
}

void
PNGRead(png_structp png, png_bytep outBytes, png_size_t byteCountToRead)
{
	struct NeStream *stm = png_get_io_ptr(png);
	E_ReadStream(stm, outBytes, byteCountToRead);
}

/* NekoEngine
 *
 * Image.c
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
