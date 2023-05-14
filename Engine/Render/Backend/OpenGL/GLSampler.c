#include "GLBackend.h"

struct NeSampler *
Re_CreateSampler(const struct NeSamplerDesc *desc)
{
	struct NeSampler *s = Sys_Alloc(1, sizeof(*s), MH_RenderBackend);
	if (!s)
		return NULL;

	s->minFilter = NeToGLTexFilter(desc->minFilter, desc->mipmapMode);
	s->magFilter = NeToGLTexFilter(desc->magFilter, desc->mipmapMode);
	s->addressModeU = NeToGLAddressMode(desc->addressModeU);
	s->addressModeV = NeToGLAddressMode(desc->addressModeV);
	s->addressModeW = NeToGLAddressMode(desc->addressModeW);
	s->anisotropicFiltering = desc->enableAnisotropy;
	s->maxAnisotropy = desc->maxAnisotropy;
	s->compare = desc->enableCompare;
	s->compareOp = NeToGLCompareOp(desc->compareOp);
	s->lodBias = desc->lodBias;
	s->minLod = desc->minLod;
	s->maxLod = desc->maxLod;

	// TODO: borderColor

	//Vk_SetSampler(Re_device, 0, s);

/*#ifdef _DEBUG
	if (desc->name)
		Vkd_SetObjectName(Re_device->dev, s, VK_OBJECT_TYPE_SAMPLER, desc->name);
#endif*/

	return s;
}

void
Re_DestroySampler(struct NeSampler *s)
{
	Sys_Free(s);
}

void
GLBk_SetSampler(struct NeTexture *tex, const struct NeSampler *s)
{
	if (!memcmp(&tex->sampler, s, sizeof(tex->sampler)))
		return;

	if (GLAD_GL_ARB_direct_state_access) {
		GL_TRACE(glTextureParameteri(tex->id, GL_TEXTURE_MIN_FILTER, s->minFilter));
		GL_TRACE(glTextureParameteri(tex->id, GL_TEXTURE_MAG_FILTER, s->magFilter));
		GL_TRACE(glTextureParameterf(tex->id, GL_TEXTURE_MAX_ANISOTROPY, s->anisotropicFiltering ? s->maxAnisotropy : 0.f));

		GL_TRACE(glTextureParameteri(tex->id, GL_TEXTURE_WRAP_S, s->addressModeU));
		GL_TRACE(glTextureParameteri(tex->id, GL_TEXTURE_WRAP_T, s->addressModeV));
		GL_TRACE(glTextureParameteri(tex->id, GL_TEXTURE_WRAP_R, s->addressModeW));

		if (s->compare) {
			//GL_TRACE(glTextureParameteri(tex->id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
			GL_TRACE(glTextureParameteri(tex->id, GL_TEXTURE_COMPARE_FUNC, s->compareOp));
		}

		GL_TRACE(glTextureParameterf(tex->id, GL_TEXTURE_LOD_BIAS, s->lodBias));
		GL_TRACE(glTextureParameterf(tex->id, GL_TEXTURE_MIN_LOD, s->minLod));
		GL_TRACE(glTextureParameterf(tex->id, GL_TEXTURE_MAX_LOD, s->maxLod));
	} else {
		GL_TRACE(glBindTexture(tex->bindPoint, tex->id));

		GL_TRACE(glTexParameteri(tex->bindPoint, GL_TEXTURE_MIN_FILTER, s->minFilter));
		GL_TRACE(glTexParameteri(tex->bindPoint, GL_TEXTURE_MAG_FILTER, s->magFilter));
		GL_TRACE(glTexParameterf(tex->bindPoint, GL_TEXTURE_MAX_ANISOTROPY, s->anisotropicFiltering ? s->maxAnisotropy : 0.f));

		GL_TRACE(glTexParameteri(tex->bindPoint, GL_TEXTURE_WRAP_S, s->addressModeU));
		GL_TRACE(glTexParameteri(tex->bindPoint, GL_TEXTURE_WRAP_T, s->addressModeV));
		GL_TRACE(glTexParameteri(tex->bindPoint, GL_TEXTURE_WRAP_R, s->addressModeW));

		if (s->compare) {
			//GL_TRACE(glTexParameteri(tex->bindPoint, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
			GL_TRACE(glTexParameteri(tex->bindPoint, GL_TEXTURE_COMPARE_FUNC, s->compareOp));
		}

		GL_TRACE(glTexParameterf(tex->bindPoint, GL_TEXTURE_LOD_BIAS, s->lodBias));
		GL_TRACE(glTexParameterf(tex->bindPoint, GL_TEXTURE_MIN_LOD, s->minLod));
		GL_TRACE(glTexParameterf(tex->bindPoint, GL_TEXTURE_MAX_LOD, s->maxLod));

		GL_TRACE(glBindTexture(tex->bindPoint, 0));
	}

	memcpy(&tex->sampler, s, sizeof(tex->sampler));
}

/* NekoEngine
 *
 * GLSampler.c
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
