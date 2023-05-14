#ifndef NE_ASSET_NTEXTURE_H
#define NE_ASSET_NTEXTURE_H

#include <Engine/Types.h>

#define NTEXTURE_HEADER			0x000000305845544Ellu	// NTEX1
#define NTEXTURE_FOOTER			0x0000584554444E45llu	// ENDTEX
#define NTEXTURE_SEC_FOOTER		0x0054434553444E45llu	// ENDSECT
#define NTEXTURE_INFO_ID		0x464E4954u				// TINF
#define NTEXTURE_PNG_DATA_ID	0x54414450u				// PDAT
#define NTEXTURE_JPG_DATA_ID	0x5441444Au				// JDAT
#define NTEXTURE_DDS_DATA_ID	0x54414444u				// DDAT
#define NTEXTURE_TGA_DATA_ID	0x54414454u				// TDAT
#define NTEXTURE_KTX_DATA_ID	0x5441444Bu				// KDAT
#define NTEXTURE_RAW_DATA_ID	0x54414442u				// RDAT
#define NTEXTURE_HDR_DATA_ID	0x54414448u				// HDAT
#define NTEXTURE_END_ID			0x54444E45u				// ENDT

#define NTEX_GRAYSCALE	1
#define NTEX_RG			2
#define NTEX_RGBA		4

struct NTextureInfo
{
	uint16_t width;
	uint16_t height;
	uint8_t type;
	uint8_t bpc;
};

struct NTexture
{
	struct NTextureInfo info;

	union {
		uint8_t *data;
		uint64_t size;
	} png;

	union {
		uint8_t *data;
		uint64_t size;
	} jpg;

	union {
		uint8_t *data;
		uint64_t size;
	} dds;

	union {
		uint8_t *data;
		uint64_t size;
	} tga;

	union {
		uint8_t *data;
		uint64_t size;
	} ktx;

	union {
		uint8_t *data;
		uint64_t size;
	} raw;
};

#endif /* NE_ASSET_NTEXTURE_H */

/* NekoEngine
 *
 * NTexture.h
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
