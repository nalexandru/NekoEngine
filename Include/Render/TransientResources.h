#ifndef _NE_RENDER_TRANSIENT_RESOURCES_H_
#define _NE_RENDER_TRANSIENT_RESOURCES_H_

#include <Render/Types.h>
#include <Render/Device.h>

#ifdef __cplusplus
extern "C" {
#endif

bool Re_InitTransientHeap(uint64_t size);
bool Re_ResizeTransientHeap(uint64_t size);

struct NeTexture *Re_CreateTransientTexture(const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
struct NeBuffer *Re_CreateTransientBuffer(const struct NeBufferDesc *desc, NeBufferHandle location, uint64_t offset, uint64_t *size);

void Re_TermTransientHeap(void);

#ifdef __cplusplus
}
#endif

#endif /* _NE_RENDER_TRANSIENT_RESOURCES_H_ */

/* NekoEngine
 *
 * TransientResources.h
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
