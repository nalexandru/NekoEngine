#include <Render/Texture.h>
#include <Render/Device.h>
#include <Engine/Config.h>

#include "GLRender.h"

#define _TEX_S3TC	1
#define _TEX_RGTC	2
#define _TEX_BPTC	3

static const char *_textureFilter = NULL;
static int32_t *_aniso = NULL;

static GLenum _textureSizedFormat[] =
{
	GL_RGBA8,
	GL_RGBA8,
	GL_BGRA,
	GL_BGRA,
	GL_BGRA,
	GL_BGRA,
	GL_RGBA16F,
	GL_RGBA16,
	GL_RGBA32F,
	GL_RGBA32UI,
	GL_RGBA12,
	GL_RGB32F,
	GL_RGB32UI,
	GL_RG8,
	GL_RG16F,
	GL_RG16,
	GL_RG32F,
	GL_RG32UI,
	GL_R8,
	GL_R16F,
	GL_R16,
	GL_R32F,
	GL_R32UI,
	_TEX_S3TC,
	_TEX_S3TC,
	_TEX_S3TC,
	_TEX_S3TC,
	_TEX_S3TC,
	_TEX_S3TC,
	_TEX_RGTC,
	_TEX_RGTC,
	_TEX_RGTC,
	_TEX_RGTC,
	_TEX_BPTC,
	_TEX_BPTC,
	_TEX_BPTC,
	_TEX_BPTC,
	GL_ALPHA
};

static GLenum _textureFormat[] =
{
	GL_RGBA,
	GL_RGBA,
	GL_BGRA,
	GL_BGRA,
	GL_BGRA,
	GL_BGRA,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	GL_RGBA,
	GL_RGB,
	GL_RGB,
	GL_RG,
	GL_RG,
	GL_RG,
	GL_RG,
	GL_RG,
	GL_RED,
	GL_RED,
	GL_RED,
	GL_RED,
	GL_RED,
	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
	GL_COMPRESSED_RED_RGTC1_EXT,
	GL_COMPRESSED_SIGNED_RED_RGTC1_EXT,
	GL_COMPRESSED_RED_GREEN_RGTC2_EXT,
	GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT,
	GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB,
	GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,
	GL_COMPRESSED_RGBA_BPTC_UNORM_ARB,
	GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB,
	GL_ALPHA
};

static GLenum _textureType[] =
{
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_HALF_FLOAT,
	GL_UNSIGNED_SHORT,
	GL_FLOAT,
	GL_UNSIGNED_INT,
	GL_UNSIGNED_INT_2_10_10_10_REV,
	GL_FLOAT,
	GL_UNSIGNED_INT,
	GL_UNSIGNED_BYTE,
	GL_HALF_FLOAT,
	GL_UNSIGNED_SHORT,
	GL_FLOAT,
	GL_UNSIGNED_INT,
	GL_UNSIGNED_BYTE,
	GL_HALF_FLOAT,
	GL_UNSIGNED_SHORT,
	GL_FLOAT,
	GL_UNSIGNED_INT,
	// compressed
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_UNSIGNED_BYTE
};

bool
GL_InitTexture(const char *name, struct Texture *tex, Handle h)
{
	GLenum target = GL_TEXTURE_2D;
	struct TextureRenderData *trd = (struct TextureRenderData *)&tex->renderDataStart;
	uint32_t levels, size;

	switch (tex->type) {
	case TT_2D: target = GL_TEXTURE_2D; break;
	case TT_3D: target = GL_TEXTURE_3D; break;
	}

	if (Re_Device.loadLock) {
		Sys_AtomicLockWrite(Re_Device.loadLock);
		GL_MakeCurrent(Re_Device.glContext);
	}
	
	if (!_textureFilter)
		_textureFilter = E_GetCVarStr(L"Render_TextureFilter",
			(GLAD_GL_EXT_texture_filter_anisotropic || GLAD_GL_ARB_texture_filter_anisotropic) ? "Anisotropic" : "Trilinear")->str;
	
	if (!_aniso)
		_aniso = &E_GetCVarI32(L"Render_TextureAnisotropy", 16)->i32;
	
	levels = 1;
	size = tex->width > tex->height ? tex->height : tex->width;
	while (size > 1) {
		++levels;
		size /= 2;
	}

	if (GLAD_GL_ARB_direct_state_access) {
		glCreateTextures(target, 1, &trd->id);

		glTextureParameteri(trd->id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(trd->id, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (!strncmp(_textureFilter, "Bilinear", strlen(_textureFilter))) {
			glTextureParameteri(trd->id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(trd->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else if (!strncmp(_textureFilter, "Trilinear", strlen(_textureFilter))) {
			glTextureParameteri(trd->id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTextureParameteri(trd->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		} else if (!strncmp(_textureFilter, "Anisotropic", strlen(_textureFilter))) {
			glTextureParameteri(trd->id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTextureParameteri(trd->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTextureParameterf(trd->id, GL_TEXTURE_MAX_ANISOTROPY, (float)*_aniso);
		} else {
			glTextureParameteri(trd->id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(trd->id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		glTextureStorage2D(trd->id, levels, _textureSizedFormat[tex->format], tex->width, tex->height);
		glTextureSubImage2D(trd->id, 0, 0, 0, tex->width, tex->height, _textureFormat[tex->format], _textureType[tex->format], tex->data);

		glGenerateTextureMipmap(trd->id);
	} else {
		glGenTextures(1, &trd->id);
		glBindTexture(target, trd->id);
		
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		if (!strncmp(_textureFilter, "Bilinear", strlen(_textureFilter))) {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else if (!strncmp(_textureFilter, "Trilinear", strlen(_textureFilter))) {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		} else if (!strncmp(_textureFilter, "Anisotropic", strlen(_textureFilter))) {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY, (float)*_aniso);
		} else {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		
		if (GLAD_GL_SGIS_generate_mipmap && !glGenerateMipmap)
			glTexParameteri(target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
		
		glTexImage2D(target, 0, _textureSizedFormat[tex->format], tex->width, tex->height, 0, _textureFormat[tex->format], _textureType[tex->format], tex->data);
		
		if (glGenerateMipmap)
			glGenerateMipmap(target);
		
		glBindTexture(target, 0);
	}
	
	if (Re_Device.loadLock)
		Sys_AtomicUnlockWrite(Re_Device.loadLock);

	return true;
}

bool
GL_UpdateTexture(struct Texture *tex, const void *data, uint64_t offset, uint64_t size)
{
	return false;
}

void
GL_TermTexture(struct Texture *tex)
{
	struct TextureRenderData *trd = (struct TextureRenderData *)&tex->renderDataStart;

	if (Re_Device.loadLock) {
		Sys_AtomicLockWrite(Re_Device.loadLock);
		GL_MakeCurrent(Re_Device.glContext);
	}

	glDeleteTextures(1, &trd->id);

	if (Re_Device.loadLock)
		Sys_AtomicUnlockWrite(Re_Device.loadLock);
}

void
GL_InitTextureFormats(void)
{
	if (GL_ShaderSupport) {
		_textureFormat[TF_ALPHA] = GL_RED;
		_textureSizedFormat[TF_ALPHA] = GL_R8;
	}
}
