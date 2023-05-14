#include <Engine/Config.h>

#include "GLBackend.h"

static struct NeArray f_textures, f_buffers;
extern struct NeArray GLBk_fbos;

struct NeTexture *
Re_BkCreateTransientTexture(const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_RenderBackend);
	if (!tex)
		return NULL;

	tex->hash = Rt_HashMemory((uint8_t *)desc, sizeof(*desc));
	tex->transient = true;

	struct GLTransientResource tr;
	if (FindTransientResource(&f_buffers, tex->hash, &tr, sizeof(tr))) {
		tex->id = tr.id;
	} else if (!GLBk_CreateTexture(desc, tex)) {
			Sys_Free(tex);
			return NULL;
	}

	tex->transient = true;
	if (location)
		GLBk_textures[location] = tex->id;

	return tex;
}

struct NeBuffer *
Re_BkCreateTransientBuffer(const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeBuffer *buff = Sys_Alloc(1, sizeof(*buff), MH_Frame);
	if (!buff)
		return NULL;

	buff->hash = Rt_HashMemory((uint8_t *)desc, sizeof(*desc));
	buff->transient = true;

	struct GLTransientResource tr;
	if (FindTransientResource(&f_buffers, buff->hash, &tr, sizeof(tr))) {
		buff->id = tr.id;
	} else if (!GLBk_CreateBuffer(desc, buff)) {
		Sys_Free(buff);
		return NULL;
	}

	//if (location)
	//	GLBk_textures[location] = tex->id;

	return buff;
}

bool
Re_InitTransientHeap(uint64_t size)
{
	Rt_InitArray(&f_textures, 10, sizeof(struct GLTransientResource), MH_RenderBackend);
	Rt_InitArray(&f_buffers, 10, sizeof(struct GLTransientResource), MH_RenderBackend);
	Rt_InitArray(&GLBk_fbos, 10, sizeof(struct GLTransientResource), MH_RenderBackend);

	return true;
}

bool
Re_ResizeTransientHeap(uint64_t size)
{
	return false;
}

void
Re_TermTransientHeap(void)
{
	struct GLTransientResource *tr;

	Rt_ArrayForEach(tr, &GLBk_fbos) {
		GL_TRACE(glDeleteFramebuffers(1, &tr->id));
	}
	Rt_ArrayForEach(tr, &f_buffers) {
		GL_TRACE(glDeleteBuffers(1, &tr->id));
	}
	Rt_ArrayForEach(tr, &f_textures) {
		GL_TRACE(glDeleteTextures(1, &tr->id));
	}
}

void
GLBk_ReleaseTransientTexture(GLuint id, uint64_t hash)
{
	struct GLTransientResource tr = { .id = id, .hash = hash };
	Rt_ArrayAdd(&f_textures, &tr);
}

void
GLBk_ReleaseTransientBuffer(GLuint id, uint64_t hash)
{
	struct GLTransientResource tr = { .id = id, .hash = hash };
	Rt_ArrayAdd(&f_buffers, &tr);
}

/* NekoEngine
 *
 * GLTransientResources.c
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
