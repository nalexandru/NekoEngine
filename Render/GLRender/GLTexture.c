#include <Render/Texture.h>
#include <Render/Device.h>
#include <Engine/Config.h>

#include "GLRender.h"

const size_t Re_TextureRenderDataSize = sizeof(struct TextureRenderData);

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
/*	DXGI_FORMAT_BC1_UNORM,
	DXGI_FORMAT_BC1_UNORM_SRGB,
	DXGI_FORMAT_BC2_UNORM,
	DXGI_FORMAT_BC2_UNORM_SRGB,
	DXGI_FORMAT_BC3_UNORM,
	DXGI_FORMAT_BC3_UNORM_SRGB,
	DXGI_FORMAT_BC4_UNORM,
	DXGI_FORMAT_BC4_SNORM,
	DXGI_FORMAT_BC5_UNORM,
	DXGI_FORMAT_BC5_SNORM,
	DXGI_FORMAT_BC6H_UF16,
	DXGI_FORMAT_BC6H_SF16,
	DXGI_FORMAT_BC7_UNORM,
	DXGI_FORMAT_BC7_UNORM_SRGB*/
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
/*	DXGI_FORMAT_BC1_UNORM,
	DXGI_FORMAT_BC1_UNORM_SRGB,
	DXGI_FORMAT_BC2_UNORM,
	DXGI_FORMAT_BC2_UNORM_SRGB,
	DXGI_FORMAT_BC3_UNORM,
	DXGI_FORMAT_BC3_UNORM_SRGB,
	DXGI_FORMAT_BC4_UNORM,
	DXGI_FORMAT_BC4_SNORM,
	DXGI_FORMAT_BC5_UNORM,
	DXGI_FORMAT_BC5_SNORM,
	DXGI_FORMAT_BC6H_UF16,
	DXGI_FORMAT_BC6H_SF16,
	DXGI_FORMAT_BC7_UNORM,
	DXGI_FORMAT_BC7_UNORM_SRGB*/
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
/*	DXGI_FORMAT_BC1_UNORM,
	DXGI_FORMAT_BC1_UNORM_SRGB,
	DXGI_FORMAT_BC2_UNORM,
	DXGI_FORMAT_BC2_UNORM_SRGB,
	DXGI_FORMAT_BC3_UNORM,
	DXGI_FORMAT_BC3_UNORM_SRGB,
	DXGI_FORMAT_BC4_UNORM,
	DXGI_FORMAT_BC4_SNORM,
	DXGI_FORMAT_BC5_UNORM,
	DXGI_FORMAT_BC5_SNORM,
	DXGI_FORMAT_BC6H_UF16,
	DXGI_FORMAT_BC6H_SF16,
	DXGI_FORMAT_BC7_UNORM,
	DXGI_FORMAT_BC7_UNORM_SRGB*/
};

bool
Re_InitTexture(const char *name, struct Texture *tex, Handle h)
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

	glCreateTextures(target, 1, &trd->id);

	if (!_textureFilter)
		_textureFilter = E_GetCVarStr(L"Render_TextureFilter", "Anisotropic")->str;

	if (!_aniso)
		_aniso = &E_GetCVarI32(L"Render_TextureAnisotropy", 16)->i32;

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

	levels = 1;
	size = tex->width > tex->height ? tex->height : tex->width;
	while (size > 1) {
		++levels;
		size /= 2;
	}

	glTextureStorage2D(trd->id, levels, _textureSizedFormat[tex->format], tex->width, tex->height);
	glTextureSubImage2D(trd->id, 0, 0, 0, tex->width, tex->height, _textureFormat[tex->format], _textureType[tex->format], tex->data);

	glGenerateTextureMipmap(trd->id);

	if (Re_Device.loadLock)
		Sys_AtomicUnlockWrite(Re_Device.loadLock);

	return true;
}

bool
Re_UpdateTexture(struct Texture *tex, const void *data, uint64_t offset, uint64_t size)
{
	return false;
}

void
Re_TermTexture(struct Texture *tex)
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
