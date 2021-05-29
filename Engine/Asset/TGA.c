#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <System/Endian.h>
#include <System/Memory.h>
#include <Render/Render.h>

#pragma pack(push, 1)

struct TGAHeader
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
_LoadUncompressed(uint8_t *dst, const uint8_t *src, const struct TGAHeader *hdr)
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
_LoadCompressed(uint8_t *dst, const uint8_t *src, const struct TGAHeader *hdr)
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
E_LoadTGAAsset(struct Stream *stm, struct TextureCreateInfo *tci)
{
	uint8_t *data = NULL;
	uint32_t imgSize, dataSize;
	struct TGAHeader hdr;

	E_ReadStream(stm, &hdr, sizeof(hdr));
	E_StreamSeek(stm, hdr.identSize, IO_SEEK_CUR);

	dataSize = (uint32_t)E_StreamLength(stm) - (uint32_t)sizeof(hdr) - hdr.identSize;

	tci->desc.mipLevels = tci->desc.arrayLayers = 1;

	if (hdr.imageType != IT_COMPRESSED && hdr.imageType != IT_UNCOMPRESSED && hdr.imageType != IT_UNCOMPRESSED_BW)
		return false;

	if (hdr.bits != 8 && hdr.bits != 24 && hdr.bits != 32)
		return false;

	if (Sys_BigEndian()) {
		hdr.colorMapStart = Sys_SwapInt16(hdr.colorMapStart);
		hdr.colorMapLength = Sys_SwapInt16(hdr.colorMapLength);
		hdr.xStart = Sys_SwapInt16(hdr.xStart);
		hdr.yStart = Sys_SwapInt16(hdr.yStart);
		hdr.width = Sys_SwapUint16(hdr.width);
		hdr.height = Sys_SwapUint16(hdr.height);
	}

	imgSize = (uint32_t)hdr.width * (uint32_t)hdr.height;
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
		_LoadCompressed(tci->data, data, &hdr);
	else
		_LoadUncompressed(tci->data, data, &hdr);

	tci->desc.width = hdr.width;
	tci->desc.height = hdr.height;
	tci->desc.format = hdr.bits == 8 ? TF_R8_UNORM : TF_R8G8B8A8_UNORM;
	
	if (data != stm->ptr)
		Sys_Free(data);

	return true;
}
