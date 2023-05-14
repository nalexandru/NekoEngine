#include <Asset/NTexture.h>
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Render/Render.h>
#include <System/Log.h>

#define NTEX_MOD	"NTexture"

bool
Asset_LoadTexture(struct NeStream *stm, struct NeTextureCreateInfo *tci)
{
	ASSET_READ_INIT();
	ASSET_CHECK_GUARD(NTEXTURE_HEADER);

	struct NTextureInfo info;
	while (!E_EndOfStream(stm)) {
		ASSET_READ_ID();

		bool rc = false;

		if (a.id == NTEXTURE_INFO_ID) {
			if (a.size != sizeof(info))
				goto error;

			if (E_ReadStream(stm, &info, sizeof(info)) != sizeof(info))
				goto error;
		} else if (a.id == NTEXTURE_RAW_DATA_ID) {
			if (info.type == NTEX_RGBA) {
				switch (info.bpc) {
				case 8: tci->desc.format = TF_R8G8B8A8_UNORM; break;
				case 16: tci->desc.format = TF_R16G16B16A16_UNORM; break;
				case 32: tci->desc.format = TF_R32G32B32A32_UINT; break;
				}
			} else if (info.type == NTEX_RG) {
				switch (info.bpc) {
				case 8: tci->desc.format = TF_R8G8_UNORM; break;
				case 16: tci->desc.format = TF_R16G16_UNORM; break;
				case 32: tci->desc.format = TF_R32G32_UINT; break;
				}
			} else if (info.type == NTEX_GRAYSCALE) {
				switch (info.bpc) {
				case 8: tci->desc.format = TF_R8_UNORM; break;
				case 16: tci->desc.format = TF_R16_UNORM; break;
				case 32: tci->desc.format = TF_R32_UINT; break;
				}
			}

			tci->dataSize = a.size;
			tci->data = Sys_Alloc(a.size, 1, MH_Asset);

			rc = E_ReadStream(stm, tci->data, tci->dataSize) == tci->dataSize;
		} else if (a.id == NTEXTURE_DDS_DATA_ID) {
			rc = Asset_LoadDDS(stm, tci);
		} else if (a.id == NTEXTURE_PNG_DATA_ID) {
			rc = Asset_LoadPNG(stm, tci);
		} else if (a.id == NTEXTURE_JPG_DATA_ID) {
			rc = Asset_LoadJPG(stm, tci);
		} else if (a.id == NTEXTURE_TGA_DATA_ID) {
			rc = Asset_LoadTGA(stm, tci);
		} else if (a.id == NTEXTURE_KTX_DATA_ID) {
		//	rc = Asset_LoadPNG(stm, tci);
		} else if (a.id == NTEXTURE_HDR_DATA_ID) {
			rc = Asset_LoadHDR(stm, tci);
		} else if (a.id == NTEXTURE_END_ID) {
			E_SeekStream(stm, -((int64_t)sizeof(a)), IO_SEEK_CUR);
			break;
		} else {
			Sys_LogEntry(NTEX_MOD, LOG_WARNING, "Unknown section id = 0x%x, size = %d", a.id, a.size);
			E_SeekStream(stm, a.size, IO_SEEK_CUR);
		}

		if (!rc) {
			Sys_LogEntry(NTEX_MOD, LOG_CRITICAL, "Failed to load data");
			goto error;
		}

		ASSET_CHECK_GUARD(NTEXTURE_SEC_FOOTER);
	}

	ASSET_CHECK_GUARD(NTEXTURE_FOOTER);

	return true;

error:
	return false;
}

/* NekoEngine
 *
 * NTexture.c
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
