#ifndef _NE_RENDER_RENDER_H_
#define _NE_RENDER_RENDER_H_

#include <Render/Types.h>
#include <Render/Core.h>
#include <Render/Device.h>
#include <Render/Context.h>
#include <Render/TransientResources.h>

#include <Render/Model.h>
#include <Render/Material.h>
#include <Render/DestroyResource.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RE_NUM_FRAMES			3
#define RE_SWAPCHAIN_TEXTURE	0

ENGINE_API extern uint32_t Re_frameId;
ENGINE_API extern struct NeSurface *Re_surface;

bool Re_InitRender(void);
void Re_RenderFrame(void);
void Re_RenderScene(struct NeScene *scn, struct NeCamera *cam, struct NeRenderGraph *graph, const struct NeTextureDesc *desc, struct NeTexture *target);
void Re_TermRender(void);

#ifdef __cplusplus
}
#endif

#endif /* _NE_RENDER_RENDER_H_ */

/* NekoEngine
 *
 * Render.h
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
