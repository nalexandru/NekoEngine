#include <stdlib.h>

#include <UI/Font.h>
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Render/Render.h>
#include <System/Endian.h>

#define FNT_MAGIC	0xB00B5000

struct NeFontHeader
{
	uint32_t magic;
	uint32_t texSize;
	uint32_t glyphCount;
};

bool
E_LoadFontAsset(struct NeStream *stm, struct NeFont *fnt)
{
	struct NeFontHeader hdr;

	if (E_ReadStream(stm, &hdr, sizeof(hdr)) != sizeof(hdr))
		return false;

	if (Sys_BigEndian()) {
		hdr.magic = Sys_SwapUint32(hdr.magic);
		hdr.texSize = Sys_SwapUint32(hdr.texSize);
		hdr.glyphCount = Sys_SwapUint32(hdr.glyphCount);
	}

	if (hdr.magic != FNT_MAGIC)
		return false;

	fnt->glyphCount = hdr.glyphCount;

	fnt->glyphs = Sys_Alloc(fnt->glyphCount, sizeof(*fnt->glyphs), MH_Asset);
	if (E_ReadStream(stm, fnt->glyphs, sizeof(*fnt->glyphs) * fnt->glyphCount) != sizeof(*fnt->glyphs) * fnt->glyphCount)
		goto error;

	if (Sys_BigEndian()) {
		for (uint32_t i = 0; i < fnt->glyphCount; ++i) {
			fnt->glyphs[i].u = Sys_SwapFloat(fnt->glyphs[i].u);
			fnt->glyphs[i].v = Sys_SwapFloat(fnt->glyphs[i].v);
			fnt->glyphs[i].tw = Sys_SwapFloat(fnt->glyphs[i].tw);
			fnt->glyphs[i].th = Sys_SwapFloat(fnt->glyphs[i].th);
			
			fnt->glyphs[i].bearing.x = Sys_SwapInt32(fnt->glyphs[i].bearing.x);
			fnt->glyphs[i].bearing.y = Sys_SwapInt32(fnt->glyphs[i].bearing.y);
			fnt->glyphs[i].size.w = Sys_SwapInt32(fnt->glyphs[i].size.w);
			fnt->glyphs[i].size.h = Sys_SwapInt32(fnt->glyphs[i].size.h);
			fnt->glyphs[i].adv = Sys_SwapInt32(fnt->glyphs[i].adv);
		}
	}
	
	struct NeTextureCreateInfo tci =
	{
		.desc.depth = 1,
		.desc.type = TT_2D,
		.desc.usage = TU_SAMPLED | TU_TRANSFER_DST,
		.desc.format = TF_R8_UNORM,
		.desc.arrayLayers = 1,
		.desc.mipLevels = 1,
		.desc.gpuOptimalTiling = true,
		.desc.memoryType = MT_GPU_LOCAL,
		.keepData = false
	};

	E_LoadImageAssetComp(stm, &tci, false, 1);

	fnt->texture = E_CreateResource("__SysFont_Texture", RES_TEXTURE, &tci);
	if (fnt->texture == E_INVALID_HANDLE)
		goto error;

	return true;

error:
	Sys_Free(fnt->glyphs);

	memset(fnt, 0x0, sizeof(*fnt));

	return false;
}

/* NekoEngine
 *
 * Font.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
