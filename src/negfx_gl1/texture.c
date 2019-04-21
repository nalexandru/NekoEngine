/* NekoEngine
 *
 * texture.c
 * Author: Alexandru Naiman
 *
 * NekoEngine OpenGL 1 Texture
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 */

#include <stdlib.h>

#include <system/log.h>

#include <gl1gfx.h>
#include <texture.h>

#define GL_TEXTURE_MODULE	"OpenGL1_Texture"

static GLenum _ne_to_gl_target[NE_TEXTURE_TYPE_COUNT] =
{
	GL_TEXTURE_1D,
	GL_TEXTURE_2D,
	0,
	0
};

static GLint _ne_to_gl_int_format[NE_IMAGE_FORMAT_COUNT] =
{
	0,
	GL_ALPHA,
	GL_ALPHA,
	GL_RGB,
	GL_RGB,
	GL_SRGB,
	GL_RGBA8,
	GL_RGBA8,
	GL_RGBA16,
	GL_RGBA16,
	GL_RGBA16,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	0, //VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	0, //VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	0, //VK_FORMAT_BC2_UNORM_BLOCK,
	0, //VK_FORMAT_BC2_SRGB_BLOCK,
	0, //VK_FORMAT_BC3_UNORM_BLOCK,
	0, //VK_FORMAT_BC3_SRGB_BLOCK,
	0, //VK_FORMAT_BC4_UNORM_BLOCK,
	0, //VK_FORMAT_BC4_SNORM_BLOCK,
	0, //VK_FORMAT_BC5_UNORM_BLOCK,
	0, //VK_FORMAT_BC5_SNORM_BLOCK,
	0, //VK_FORMAT_BC6H_UFLOAT_BLOCK,
	0, //VK_FORMAT_BC6H_SFLOAT_BLOCK,
	0, //VK_FORMAT_BC7_UNORM_BLOCK,
	0, //VK_FORMAT_BC7_SRGB_BLOCK
};

static GLint _ne_to_gl_format[NE_IMAGE_FORMAT_COUNT] =
{
	0,
	GL_ALPHA,
	GL_ALPHA,
	GL_RGB,
	GL_RGB,
	GL_RGB,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	0, //VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	0, //VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	0, //VK_FORMAT_BC2_UNORM_BLOCK,
	0, //VK_FORMAT_BC2_SRGB_BLOCK,
	0, //VK_FORMAT_BC3_UNORM_BLOCK,
	0, //VK_FORMAT_BC3_SRGB_BLOCK,
	0, //VK_FORMAT_BC4_UNORM_BLOCK,
	0, //VK_FORMAT_BC4_SNORM_BLOCK,
	0, //VK_FORMAT_BC5_UNORM_BLOCK,
	0, //VK_FORMAT_BC5_SNORM_BLOCK,
	0, //VK_FORMAT_BC6H_UFLOAT_BLOCK,
	0, //VK_FORMAT_BC6H_SFLOAT_BLOCK,
	0, //VK_FORMAT_BC7_UNORM_BLOCK,
	0, //VK_FORMAT_BC7_SRGB_BLOCK
};

static GLint _ne_to_gl_type[NE_IMAGE_FORMAT_COUNT] =
{
	0,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	2,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_SHORT,
	GL_FLOAT,
	GL_UNSIGNED_INT,
	GL_FLOAT,
	GL_UNSIGNED_INT,
	GL_FLOAT,
	0, //VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	0, //VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	0, //VK_FORMAT_BC2_UNORM_BLOCK,
	0, //VK_FORMAT_BC2_SRGB_BLOCK,
	0, //VK_FORMAT_BC3_UNORM_BLOCK,
	0, //VK_FORMAT_BC3_SRGB_BLOCK,
	0, //VK_FORMAT_BC4_UNORM_BLOCK,
	0, //VK_FORMAT_BC4_SNORM_BLOCK,
	0, //VK_FORMAT_BC5_UNORM_BLOCK,
	0, //VK_FORMAT_BC5_SNORM_BLOCK,
	0, //VK_FORMAT_BC6H_UFLOAT_BLOCK,
	0, //VK_FORMAT_BC6H_SFLOAT_BLOCK,
	0, //VK_FORMAT_BC7_UNORM_BLOCK,
	0, //VK_FORMAT_BC7_SRGB_BLOCK
};

struct ne_texture *
gl1gfx_create_texture(
	struct ne_texture_create_info *ci,
	const void *data,
	uint64_t size)
{
	struct ne_texture *tex = calloc(1, sizeof(struct ne_texture));
	
	tex->target = _ne_to_gl_target[ci->type];
	tex->format = _ne_to_gl_format[ci->format];
	tex->int_format = _ne_to_gl_int_format[ci->format];
	tex->type = _ne_to_gl_type[ci->format];
	tex->width = ci->width;
	tex->height = ci->height;
	tex->depth = ci->depth;
	tex->levels = ci->levels;
	tex->layers = ci->layers;

	glGenTextures(1, &tex->id);
	
	log_entry(GL_TEXTURE_MODULE, LOG_INFORMATION, "created texture");
	
	if (data)
		gl1gfx_upload_image(tex, data, size);
	
	return tex;
}

ne_status
gl1gfx_upload_image(
	struct ne_texture *tex,
	const void *data,
	uint64_t size)
{
	GLenum err;
	
	glGetError();
	
	log_entry(GL_TEXTURE_MODULE, LOG_INFORMATION, "uploading texture");
		
	glBindTexture(tex->target, tex->id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	if (tex->int_format == GL_ALPHA)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glTexImage2D(tex->target, 0, tex->int_format, tex->width, tex->height, 0,
				tex->format, tex->type, data);
	
	if (tex->int_format == GL_ALPHA)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	
//	glGenerateMipmap(GL_TEXTURE_2D);
	
	if ((err = glGetError()) != GL_NO_ERROR) {
		log_entry(GL_TEXTURE_MODULE, LOG_CRITICAL,
				"Failed to upload texture: 0x%x", err);
		return NE_FAIL;
	}
	
	glBindTexture(tex->target, 0);
	
	return NE_OK;
}

void
gl1gfx_destroy_texture(struct ne_texture *tex)
{
	glDeleteTextures(1, &tex->id);
	free(tex);
}

