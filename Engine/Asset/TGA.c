#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <System/Endian.h>
#include <System/Memory.h>
#include <Render/Render.h>

#pragma pack(push, 1)

struct NeTGAHeader
{
	uint8_t identSize;
	uint8_t colorMapType;
	uint8_t imageType;
	int16_t colorMapStart;
	int16_t colorMapLength;
	uint8_t colorMapBits;
	int16_t xStart;
	int16_t yStart;
	uint16_t width;
	uint16_t height;
	uint8_t bits;
	uint8_t descriptor;
};

#pragma pack(pop)

#define IT_COMPRESSED		10
#define IT_UNCOMPRESSED		2
#define IT_UNCOMPRESSED_BW	3

static inline void
LoadUncompressed(uint8_t *dst, const uint8_t *src, const struct NeTGAHeader *hdr)
{
	int32_t i, j;
	int64_t w = hdr->width;
	int64_t h = hdr->height;
	int64_t rowSize = w * hdr->bits / 8;
	bool inverted = ((hdr->descriptor & (1 << 5)) != 0);
	const uint8_t *src_row;

	for (i = 0; i < h; i++) {
		src_row = src + (inverted ? (h - i - 1) * rowSize : i * rowSize);
		if (hdr->bits == 8) {
			for (j = 0; j < w; ++j)
				*dst++ = *src_row++;
		} else if (hdr->bits == 24) {
			for (j = 0; j < w; ++j) {
				*dst++ = src_row[2];
				*dst++ = src_row[1];
				*dst++ = src_row[0];
				*dst++ = 0;
				src_row += 3;
			}
		} else {
			for (j = 0; j < w; ++j) {
				*dst++ = src_row[2];
				*dst++ = src_row[1];
				*dst++ = src_row[0];
				*dst++ = src_row[3];
				src_row += 4;
			}
		}
	}
}

static inline void
LoadCompressed(uint8_t *dst, const uint8_t *src, const struct NeTGAHeader *hdr)
{
	int64_t w = hdr->width;
	int64_t h = hdr->height;
	int64_t rowSize = w * hdr->bits / 8;
	bool inverted = ((hdr->descriptor & (1 << 5)) != 0);
	uint8_t *dstPtr = inverted ? dst + (h + 1) * rowSize : dst;
	int64_t count = 0;
	int64_t pixels = w * h;
	uint8_t chunk;
	int32_t chunkSize;
	int32_t i;

	while (pixels > count) {
		chunk = *src++;
		if (chunk < 128) {
			chunkSize = chunk + 1;
			for (i = 0; i < chunkSize; i++) {
				if (inverted && (count % w) == 0)
					dstPtr -= 2 * rowSize;
				*dstPtr++ = src[2];
				*dstPtr++ = src[1];
				*dstPtr++ = src[0];
				src += 3;
				*dstPtr++ = hdr->bits != 24 ? *src++ : 0;
				++count;
			}
		} else {
			chunkSize = chunk - 127;
			for (i = 0; i < chunkSize; i++) {
				if (inverted && (count % w) == 0)
					dstPtr -= 2 * rowSize;
				*dstPtr++ = src[2];
				*dstPtr++ = src[1];
				*dstPtr++ = src[0];
				*dstPtr++ = hdr->bits != 24 ? src[3] : 0;
				++count;
			}
			src += (hdr->bits >> 3);
		}
	}
}

bool
Asset_LoadTGA(struct NeStream *stm, struct NeTextureCreateInfo *tci)
{
	uint8_t *data = NULL;
	uint64_t imgSize, dataSize;
	struct NeTGAHeader hdr;

	E_ReadStream(stm, &hdr, sizeof(hdr));
	E_SeekStream(stm, hdr.identSize, IO_SEEK_CUR);

	dataSize = E_StreamLength(stm) - sizeof(hdr) - hdr.identSize;

	tci->desc.mipLevels = tci->desc.arrayLayers = 1;

	if (hdr.imageType != IT_COMPRESSED && hdr.imageType != IT_UNCOMPRESSED && hdr.imageType != IT_UNCOMPRESSED_BW)
		return false;

	if (hdr.bits != 8 && hdr.bits != 24 && hdr.bits != 32)
		return false;

#ifdef SYS_BIG_ENDIAN
	hdr.colorMapStart = Sys_SwapInt16(hdr.colorMapStart);
	hdr.colorMapLength = Sys_SwapInt16(hdr.colorMapLength);
	hdr.xStart = Sys_SwapInt16(hdr.xStart);
	hdr.yStart = Sys_SwapInt16(hdr.yStart);
	hdr.width = Sys_SwapUint16(hdr.width);
	hdr.height = Sys_SwapUint16(hdr.height);
#endif

	imgSize = (uint64_t)hdr.width * (uint64_t)hdr.height;
	if (imgSize < hdr.width || imgSize < hdr.height)
		return false;

	tci->dataSize = imgSize * (hdr.bits == 8 ? 1 : 4);
	if (tci->dataSize < imgSize)
		return false;

	tci->data = Sys_Alloc(sizeof(uint8_t), tci->dataSize, MH_Asset);
	if (!tci->data)
		return false;

	data = stm->ptr;
	if (!data) {
		data = Sys_Alloc(sizeof(uint8_t), dataSize, MH_Asset);
		if (!data) {
			Sys_Free(tci->data);
			return false;
		}

		E_ReadStream(stm, data, dataSize);
	}

	if (hdr.imageType == IT_COMPRESSED)
		LoadCompressed(tci->data, data, &hdr);
	else
		LoadUncompressed(tci->data, data, &hdr);

	tci->desc.width = hdr.width;
	tci->desc.height = hdr.height;
	tci->desc.format = hdr.bits == 8 ? TF_R8_UNORM : TF_R8G8B8A8_UNORM;
	
	if (data != stm->ptr)
		Sys_Free(data);

	return true;
}

/* NekoEngine
 *
 * TGA.c
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
