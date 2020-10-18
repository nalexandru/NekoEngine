#include <Render/Texture.h>
#include <Render/Device.h>
#include <System/Endian.h>

#include "D3D9Render.h"

const size_t Re_TextureRenderDataSize = sizeof(struct TextureRenderData);

enum FmtSwap
{
	FS_UNKNOWN,
	FS_RGBA,
	FS_BGRA,
	FS_BGRX,
	FS_RGBA16,
	FS_RG16,
	FS_RGBA16F,
	FS_RG16F,
	FS_RG32F
};

struct D3D9Format
{
	D3DFORMAT format;
	enum FmtSwap swap;
	size_t byteSize;
};

static D3D9Format _textureFormat[] =
{
	{ D3DFMT_A8R8G8B8, FS_RGBA, 4 },
	{ D3DFMT_A8R8G8B8, FS_RGBA, 4 },
	{ D3DFMT_A8R8G8B8, FS_BGRA, 4 },
	{ D3DFMT_A8R8G8B8, FS_BGRA, 4 },
	{ D3DFMT_X8R8G8B8, FS_BGRX, 4 },
	{ D3DFMT_X8R8G8B8, FS_BGRX, 4 },
	{ D3DFMT_A16B16G16R16F, FS_RGBA16F, 8 },
	{ D3DFMT_A16B16G16R16, FS_RGBA16, 8 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_G16R16, FS_RG16, 4 },
	{ D3DFMT_G16R16F, FS_RG16F, 4 },
	{ D3DFMT_G32R32F, FS_RG32F, 8 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_L8, FS_UNKNOWN, 1 },
	{ D3DFMT_R16F, FS_UNKNOWN, 2 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_R32F, FS_UNKNOWN, 4 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_DXT1, FS_UNKNOWN, 0 },
	{ D3DFMT_DXT1, FS_UNKNOWN, 0 },
	{ D3DFMT_DXT2, FS_UNKNOWN, 0 },
	{ D3DFMT_DXT2, FS_UNKNOWN, 0 },
	{ D3DFMT_DXT3, FS_UNKNOWN, 0 },
	{ D3DFMT_DXT3, FS_UNKNOWN, 0 },
	{ D3DFMT_DXT4, FS_UNKNOWN, 0 },
	{ D3DFMT_DXT4, FS_UNKNOWN, 0 },
	{ D3DFMT_DXT5, FS_UNKNOWN, 0 },
	{ D3DFMT_DXT5, FS_UNKNOWN, 0 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_UNKNOWN, FS_UNKNOWN, 0 },
	{ D3DFMT_L8, FS_UNKNOWN, 1 }
};

bool
D3D9_InitTexture(const char *name, struct Texture *tex, Handle h)
{
	HRESULT hr;
	IDirect3DTexture9 *staging = NULL;
	struct TextureRenderData *trd = (struct TextureRenderData *)&tex->renderDataStart;

	if (Re_Device.caps.MaxTextureWidth < tex->width || Re_Device.caps.MaxTextureHeight < tex->height)
		return false;

	D3D9Format fmt = _textureFormat[tex->format];
	if (fmt.format == D3DFMT_UNKNOWN)
		return false;

	hr = Re_Device.dev->CreateTexture(tex->width, tex->height, 1, 0, fmt.format, D3DPOOL_SYSTEMMEM, &staging, NULL);
	if (FAILED(hr))
		return false;

	size_t px = (size_t)tex->width * tex->height;
	size_t size = (size_t)tex->width * tex->height * fmt.byteSize;

	D3DLOCKED_RECT rect;
	hr = staging->LockRect(0, &rect, NULL, D3DLOCK_DISCARD);
	if (FAILED(hr))
		goto error;

	if (fmt.swap == FS_RGBA) {
		for (size_t i = 0; i < px; ++i)
			((uint32_t *)rect.pBits)[i] = Sys_SwapUint32(((uint32_t *)tex->data)[i]) >> 8 | 0xFF000000;
	} else if (fmt.swap == FS_BGRA || fmt.swap == FS_BGRX) {

	} else if (fmt.swap == FS_RGBA16 || fmt.swap == FS_RGBA16F) {

	} else if (fmt.swap == FS_RG16 || fmt.swap == FS_RG16F) {

	} else if (fmt.swap == FS_RG32F) {

	} else {
		memcpy(rect.pBits, tex->data, size);
	}

	staging->UnlockRect(0);

	hr = Re_Device.dev->CreateTexture(tex->width, tex->height, 0, D3DUSAGE_AUTOGENMIPMAP, fmt.format, D3DPOOL_DEFAULT, &trd->tex, NULL);
	if (FAILED(hr))
		goto error;

	Re_Device.dev->UpdateTexture(staging, trd->tex);

	staging->Release();

	return true;

error:
	if (staging)
		staging->Release();

	if (trd->tex)
		trd->tex->Release();

	return false;
}

bool
D3D9_UpdateTexture(struct Texture *tex, const void *data, uint64_t offset, uint64_t size)
{
	return false;
}

void
D3D9_TermTexture(struct Texture *tex)
{
	struct TextureRenderData *trd = (struct TextureRenderData *)&tex->renderDataStart;
	trd->tex->Release();
}