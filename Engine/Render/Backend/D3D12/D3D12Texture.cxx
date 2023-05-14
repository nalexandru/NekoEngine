#include <stdlib.h>

#include "D3D12Backend.h"

struct NeTexture *
Re_BkCreateTexture(const struct NeTextureDesc *desc, uint16_t location)
{
	NeTexture *tex = (NeTexture *)Sys_Alloc(1, sizeof(*tex), MH_RenderBackend);
	if (!tex)
		return NULL;

	tex->transient = false;

	D3D12_RESOURCE_DESC rd = NeToD3DTextureDesc(desc);
	D3D12_HEAP_PROPERTIES hp{ D3D12_HEAP_TYPE_DEFAULT };

	if (FAILED(Re_device->dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd,
														D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&tex->res)))) {
		Sys_Free(tex);
		return NULL;
	}

	D3D12Bk_SetTexture(location, tex->res);
	tex->state = D3D12_RESOURCE_STATE_COPY_DEST;

#ifdef _DEBUG
	if (desc->name)
		tex->res->SetName(NeWin32_UTF8toUCS2(desc->name));
#endif

	return tex;
}

enum NeTextureLayout
Re_BkTextureLayout(const struct NeTexture *tex)
{
	return D3DResourceStateToNeImageLayout(tex->state);
}

void
Re_BkDestroyTexture(struct NeTexture *tex)
{
	tex->res->Release();
	Sys_Free(tex);
}

/* NekoEngine
 *
 * D3D12Texture.cxx
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
