#include <stdlib.h>

#include <System/Memory.h>

#include "GLBackend.h"

struct NeArray GLBk_fbos;

struct NeFramebuffer *
Re_CreateFramebuffer(const struct NeFramebufferDesc *desc)
{
	struct NeFramebuffer *fb = Sys_Alloc(1, sizeof(*fb), MH_Frame);
	if (!fb)
		return NULL;

	fb->hash = Rt_HashMemory((uint8_t *)desc, sizeof(*desc));

	struct GLTransientResource tr;
	if (FindTransientResource(&GLBk_fbos, fb->hash, &tr, sizeof(tr))) {
		fb->id = tr.id;
	} else {
		if (GLAD_GL_ARB_direct_state_access) {
			GL_TRACE(glCreateFramebuffers(1, &fb->id));
		} else {
			GL_TRACE(glGenFramebuffers(1, &fb->id));
		}
		if (!fb->id)
			return NULL;
	}

#ifdef _DEBUG
//	if (desc->name)
//		Vkd_SetObjectName(Re_device->dev, fb->fb, VK_OBJECT_TYPE_FRAMEBUFFER, desc->name);
#endif

	return fb;
}

void
Re_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex)
{
	if (GLAD_GL_ARB_direct_state_access) {
		GL_TRACE(glNamedFramebufferTexture2DEXT(fb->id, GL_COLOR_ATTACHMENT0 + pos, tex->bindPoint, tex->id, 0));
	} else {
		GL_TRACE(glBindFramebuffer(GL_FRAMEBUFFER, fb->id));
		GL_TRACE(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex->id, 0));
		GL_TRACE(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
}

void
Re_DestroyFramebuffer(struct NeFramebuffer *fb)
{
	struct GLTransientResource tr = { .id = fb->id, .hash = fb->hash };
	Rt_ArrayAdd(&GLBk_fbos, &tr);

	Sys_Free(fb);
}

/* NekoEngine
 *
 * GLFramebuffer.c
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
