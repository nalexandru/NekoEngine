#ifndef _NE_UI_INTERNAL_H_
#define _NE_UI_INTERNAL_H_

#include <UI/UI.h>
#include <UI/Font.h>
#include <UI/Text.h>
#include <Runtime/Runtime.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,1)
struct NeUIVertex
{
	float posUv[4];
	float color[4];
};
#pragma pack(pop)

struct NeUIDrawCmd
{
	uint16_t vtxOffset;
	uint16_t vtxCount;
	uint16_t idxOffset;
	uint16_t idxCount;
	NeHandle texture;
};

extern struct NeFont UI_sysFont;
extern NeBufferHandle UI_vertexBuffer, UI_indexBuffer;
extern uint64_t UI_vertexBufferSize, UI_indexBufferSize;
extern struct NeArray UI_standaloneContexts;

void _UI_DrawContext(void **comp, void *a);

#ifdef __cplusplus
}
#endif

#endif /* _NE_UI_INTERNAL_H_ */

/* NekoEngine
 *
 * Internal.h
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
