#include <stdlib.h>

#include "GLBackend.h"

GLuint GLBk_textures[UINT16_MAX];

bool
GLBk_CreateTexture(const struct NeTextureDesc *desc, struct NeTexture *tex)
{
	switch (desc->type) {
		case TT_2D: tex->bindPoint = GL_TEXTURE_2D; break;
		case TT_3D: tex->bindPoint = GL_TEXTURE_3D; break;
		case TT_Cube: tex->bindPoint = GL_TEXTURE_CUBE_MAP; break;
		case TT_2D_Multisample: tex->bindPoint = GL_TEXTURE_2D_MULTISAMPLE; break;
	}

	if (GLAD_GL_ARB_direct_state_access) {
		GL_TRACE(glCreateTextures(tex->bindPoint, 1, &tex->id));
		if (!tex->id)
			return false;

		switch (desc->type) {
			case TT_2D: GL_TRACE(glTextureStorage2D(tex->id, desc->mipLevels, NeToGLSizedTextureFormat(desc->format), desc->width, desc->height)); break;
			case TT_3D: GL_TRACE(glTextureStorage3D(tex->id, desc->mipLevels, NeToGLSizedTextureFormat(desc->format), desc->width, desc->height, desc->depth)); break;
			case TT_Cube: GL_TRACE(glTextureStorage2D(tex->id, desc->mipLevels, NeToGLSizedTextureFormat(desc->format), desc->width, desc->height)); break;
			case TT_2D_Multisample: GL_TRACE(glTextureStorage2DMultisample(tex->id, desc->mipLevels, NeToGLSizedTextureFormat(desc->format), desc->width, desc->height, false)); break;
		}
	} else {
		GL_TRACE(glGenTextures(1, &tex->id));
		if (!tex->id)
			return false;

		GL_TRACE(glBindTexture(tex->bindPoint, tex->id));

		if (GLAD_GL_ARB_texture_storage) {
			switch (desc->type) {
				case TT_2D: GL_TRACE(glTexStorage2D(tex->bindPoint, desc->mipLevels, NeToGLSizedTextureFormat(desc->format), desc->width, desc->height)); break;
				case TT_3D: GL_TRACE(glTexStorage3D(tex->bindPoint, desc->mipLevels, NeToGLSizedTextureFormat(desc->format), desc->width, desc->height, desc->depth)); break;
				case TT_Cube: GL_TRACE(glTexStorage2D(tex->bindPoint, desc->mipLevels, NeToGLSizedTextureFormat(desc->format), desc->width, desc->height)); break;
				case TT_2D_Multisample: GL_TRACE(glTexStorage2DMultisample(tex->bindPoint, desc->mipLevels, NeToGLSizedTextureFormat(desc->format), desc->width, desc->height, false)); break;
			}
		}

		GL_TRACE(glBindTexture(tex->bindPoint, 0));
	}

	if (GLAD_GL_ARB_bindless_texture) {
		tex->handle = GL_TRACE(glGetTextureHandleARB(tex->id));
		GL_TRACE(glMakeTextureHandleResidentARB(tex->handle));
	}

#ifdef _DEBUG
	//if (desc->name)
	//	Vkd_SetObjectName(Re_device->dev, buff->buff, VK_OBJECT_TYPE_BUFFER, desc->name);
#endif

	return true;
}

void
GLBk_UploadImage(struct NeTexture *tex, const struct NeBuffer *buff, struct NeBufferImageCopy *bic)
{
	if (GLAD_GL_ARB_direct_state_access) {
		if (GLBkCompressedFormat(tex->format)) {
			switch (tex->bindPoint) {
			case GL_TEXTURE_2D:
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_2D_MULTISAMPLE: {
				GL_TRACE(glCompressedTextureSubImage2D(tex->id, bic->subresource.mipLevel, bic->imageOffset.x, bic->imageOffset.y,
												bic->imageSize.width, bic->imageSize.height, tex->format, tex->type, buff->staging));
			} break;
			case GL_TEXTURE_3D: {
				GL_TRACE(glCompressedTextureSubImage3D(tex->id, bic->subresource.mipLevel, bic->imageOffset.x, bic->imageOffset.y,
									bic->imageOffset.z,	bic->imageSize.width, bic->imageSize.height, bic->imageSize.depth,
									tex->format, tex->type, buff->staging));
			} break;
			}
		} else {
			switch (tex->bindPoint) {
			case GL_TEXTURE_2D:
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_2D_MULTISAMPLE: {
				GL_TRACE(glTextureSubImage2D(tex->id, bic->subresource.mipLevel, bic->imageOffset.x, bic->imageOffset.y,
									bic->imageSize.width, bic->imageSize.height, tex->format, tex->type,
									buff->staging));
			} break;
			case GL_TEXTURE_3D: {
				GL_TRACE(glTextureSubImage3D(tex->id, bic->subresource.mipLevel, bic->imageOffset.x, bic->imageOffset.y,
									bic->imageOffset.z,	bic->imageSize.width, bic->imageSize.height, bic->imageSize.depth,
									tex->format, tex->type, buff->staging));
			} break;
			}
		}

		return;
	}

	GL_TRACE(glBindTexture(tex->bindPoint, tex->id));

	if (GLAD_GL_ARB_texture_storage) {
		if (GLBkCompressedFormat(tex->format)) {
			switch (tex->bindPoint) {
			case GL_TEXTURE_2D:
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_2D_MULTISAMPLE: {
				GL_TRACE(glCompressedTexSubImage2D(tex->bindPoint, bic->subresource.mipLevel, bic->imageOffset.x, bic->imageOffset.y,
											bic->imageSize.width, bic->imageSize.height, tex->format, tex->formatSize, buff->staging));
			} break;
			case GL_TEXTURE_3D: {
				GL_TRACE(glCompressedTexSubImage3D(tex->bindPoint, bic->subresource.mipLevel, bic->imageOffset.x, bic->imageOffset.y,
											bic->imageOffset.z,	bic->imageSize.width, bic->imageSize.height, bic->imageSize.depth,
											tex->format, tex->formatSize, buff->staging));
			} break;
			}
		} else {
			switch (tex->bindPoint) {
			case GL_TEXTURE_2D:
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_2D_MULTISAMPLE: {
				GL_TRACE(glTexSubImage2D(tex->bindPoint, bic->subresource.mipLevel, bic->imageOffset.x, bic->imageOffset.y,
									bic->imageSize.width, bic->imageSize.height, tex->format, tex->type,
									buff->staging));
			} break;
			case GL_TEXTURE_3D: {
				GL_TRACE(glTexSubImage3D(tex->bindPoint, bic->subresource.mipLevel, bic->imageOffset.x, bic->imageOffset.y,
									bic->imageOffset.z,	bic->imageSize.width, bic->imageSize.height, bic->imageSize.depth,
									tex->format, tex->type, buff->staging));
			} break;
			}
		}
	} else {
		if (GLBkCompressedFormat(tex->format)) {
			switch (tex->bindPoint) {
			case GL_TEXTURE_2D:
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_2D_MULTISAMPLE: {
				GL_TRACE(glCompressedTexImage2D(tex->bindPoint, bic->subresource.mipLevel, tex->internalFormat,
										bic->imageSize.width, bic->imageSize.height, 0, tex->formatSize, buff->staging));
			} break;
			case GL_TEXTURE_3D: {
				GL_TRACE(glCompressedTexImage3D(tex->bindPoint, bic->subresource.mipLevel, tex->internalFormat, bic->imageSize.width,
										bic->imageSize.height, bic->imageSize.depth, 0, tex->formatSize, buff->staging));
			} break;
			}
		} else {
			switch (tex->bindPoint) {
			case GL_TEXTURE_2D:
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_2D_MULTISAMPLE: {
				GL_TRACE(glTexImage2D(tex->bindPoint, bic->subresource.mipLevel, tex->internalFormat, bic->imageOffset.x,
								bic->imageOffset.y, 0, tex->format, tex->type, buff->staging));
			} break;
			case GL_TEXTURE_3D: {
				GL_TRACE(glTexImage3D(tex->bindPoint, bic->subresource.mipLevel, tex->internalFormat, bic->imageSize.width,
								bic->imageSize.height, bic->imageSize.depth, 0, tex->format, tex->type, buff->staging));
			} break;
			}
		}
	}

	GL_TRACE(glBindTexture(tex->bindPoint, 0));
}

struct NeTexture *
Re_BkCreateTexture(const struct NeTextureDesc *desc, uint16_t location)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_RenderBackend);
	if (!tex)
		return NULL;

	if (!GLBk_CreateTexture(desc, tex)) {
		Sys_Free(tex);
		return NULL;
	}

	tex->transient = false;
	GLBk_textures[location] = tex->id;

	return tex;
}

enum NeTextureLayout Re_BkTextureLayout(const struct NeTexture *tex) { return TL_UNKNOWN; }

void
Re_BkDestroyTexture(struct NeTexture *tex)
{
	if (tex->transient) {
		GLBk_ReleaseTransientTexture(tex->id, tex->hash);
		return;
	}

	if (GLAD_GL_ARB_bindless_texture) {
		GL_TRACE(glMakeTextureHandleNonResidentARB(tex->handle));
	}

	GL_TRACE(glDeleteTextures(1, &tex->id));
	Sys_Free(tex);
}

/* NekoEngine
 *
 * GLTexture.c
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
