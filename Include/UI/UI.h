#ifndef _NE_UI_UI_H_
#define _NE_UI_UI_H_

#include <UI/Text.h>
#include <Engine/Types.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_UPDATE_BUFFERS		"UI_UpdateBuffers"
#define UI_DRAW_CONTEXT			"UI_DrawContext"

struct NeUIContext
{
	NE_COMPONENT_BASE;

	struct NeArray vertices, indices, draws;
};

bool UI_InitUI(void);
void UI_TermUI(void);

void UI_Update(struct NeScene *s);

struct NeUIContext *UI_CreateStandaloneContext(uint32_t vertexCount, uint32_t indexCount, uint32_t drawCallCount);
void UI_DestroyStandaloneContext(struct NeUIContext *ctx);

bool UI_ResizeBuffers(uint32_t maxVertices, uint32_t maxIndices);

#ifdef __cplusplus
}
#endif

#endif /* _NE_UI_UI_H_ */

/* NekoEngine
 *
 * UI.h
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
