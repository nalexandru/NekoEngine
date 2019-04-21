/* NekoEngine
 *
 * tga.c
 * Author: Alexandru Naiman
 *
 * NekoEngine TGA Loader
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

#include <system/system.h>
#include <system/compat.h>
#include <runtime/runtime.h>

#include <graphics/texture.h>

#pragma pack(push, 1)

struct tga_header
{
	uint8_t ident_size;
	uint8_t color_map_type;
	uint8_t image_type;
	int16_t color_map_start;
	int16_t color_map_length;
	uint8_t color_map_bits;
	int16_t x_start;
	int16_t y_start;
	uint16_t width;
	uint16_t height;
	uint8_t bits;
	uint8_t descriptor;
};

#pragma pack(pop)

#define IT_COMPRESSED		10
#define IT_UNCOMPRESSED		2
#define IT_UNCOMPRESSED_BW	3

static INLINE void
_load_uncompressed(
	uint8_t *dst,
	const uint8_t *src,
	const struct tga_header *hdr)
{
	int w = hdr->width;
	int h = hdr->height;
	int row_size = w * hdr->bits / 8;
	bool inverted = ((hdr->descriptor & (1 << 5)) != 0);
	for (int i = 0; i < h; i++) {
		const uint8_t *src_row = src +
			(inverted ? (h - i - 1) * row_size : i * row_size);
		if (hdr->bits == 8) {
			for (int j = 0; j < w; ++j)
				*dst++ = *src_row++;
		} else if (hdr->bits == 24) {
			for (int j = 0; j < w; ++j) {
				*dst++ = src_row[2];
				*dst++ = src_row[1];
				*dst++ = src_row[0];
				*dst++ = 0;
				src_row += 3;
			}
		} else {
			for (int j = 0; j < w; ++j) {
				*dst++ = src_row[2];
				*dst++ = src_row[1];
				*dst++ = src_row[0];
				*dst++ = src_row[3];
				src_row += 4;
			}
		}
	}
}

static INLINE void
_load_compressed(
	uint8_t *dst,
	const uint8_t *src,
	const struct tga_header *hdr)
{
	int w = hdr->width;
	int h = hdr->height;
	int row_size = w * hdr->bits / 8;
	bool inverted = ((hdr->descriptor & (1 << 5)) != 0);
	uint8_t *dst_ptr = inverted ? dst + (h + 1) * row_size : dst;
	int count = 0;
	int pixels = w * h;

	while (pixels > count) {
		unsigned char chunk = *src++;
		if (chunk < 128) {
			int chunkSize = chunk + 1;
			for (int i = 0; i < chunkSize; i++) {
				if (inverted && (count % w) == 0)
					dst_ptr -= 2 * row_size;
				*dst_ptr++ = src[2];
				*dst_ptr++ = src[1];
				*dst_ptr++ = src[0];
				src += 3;
				*dst_ptr++ = hdr->bits != 24 ? *src++ : 0;
				++count;
			}
		} else {
			int chunkSize = chunk - 127;
			for (int i = 0; i < chunkSize; i++) {
				if (inverted && (count % w) == 0)
					dst_ptr -= 2 * row_size;
				*dst_ptr++ = src[2];
				*dst_ptr++ = src[1];
				*dst_ptr++ = src[0];
				*dst_ptr++ = hdr->bits != 24 ? src[3] : 0;
				++count;
			}
			src += (hdr->bits >> 3);
		}
	}
}

ne_status
asset_load_tga(
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

	struct tga_header hdr;
	memcpy(&hdr, data, sizeof(hdr));
	data += sizeof(hdr) + hdr.ident_size;

	if (hdr.image_type != IT_COMPRESSED &&
		hdr.image_type != IT_UNCOMPRESSED &&
		hdr.image_type != IT_UNCOMPRESSED_BW)
		return NE_INVALID_HEADER;

	if (hdr.bits != 8 && hdr.bits != 24 && hdr.bits != 32)
		return NE_INVALID_HEADER;

	if (sys_is_big_endian()) {
		rt_swap_int16(&hdr.color_map_start);
		rt_swap_int16(&hdr.color_map_length);
		rt_swap_int16(&hdr.x_start);
		rt_swap_int16(&hdr.y_start);
		rt_swap_uint16(&hdr.width);
		rt_swap_uint16(&hdr.height);
	}

	uint64_t img_size = (uint64_t)hdr.width * (uint64_t)hdr.height;
	if (img_size < hdr.width || img_size < hdr.height)
		return NE_INVALID_HEADER;

	uint64_t size = (uint64_t)img_size * (hdr.bits == 8 ? 8 : 32);
	if (size < img_size)
		return NE_INVALID_HEADER;

	*img_data_size = (size + 7) / 8;
	*img_data = (uint8_t *)calloc(sizeof(uint8_t), *img_data_size);

	if (hdr.image_type == IT_COMPRESSED)
		_load_compressed(*img_data, data, &hdr);
	else
		_load_uncompressed(*img_data, data, &hdr);

	*width = hdr.width;
	*height = hdr.height;
	*depth = 1;
	*format = hdr.bits == 8 ?
			NE_IMAGE_FORMAT_R8_UNORM :
			NE_IMAGE_FORMAT_R8G8B8A8_UNORM;
	*levels = 1;
	*layers = 1;
	*should_free = true;

	return NE_OK;
}

