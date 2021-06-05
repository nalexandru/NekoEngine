#include <stdlib.h>

#include <UI/Font.h>
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Render/Render.h>
#include <System/Endian.h>

#define FNT_MAGIC	0xB00B5000

struct FontHeader
{
	uint32_t magic;
	uint32_t texSize;
	uint32_t glyphCount;
};

bool
E_LoadFontAsset(struct Stream *stm, struct Font *fnt)
{
	void *texData = NULL;
	uint32_t texDataSize;
	struct FontHeader hdr;
	uint32_t texSize;

	if (E_ReadStream(stm, &hdr, sizeof(hdr)) != sizeof(hdr))
		return false;

	if (Sys_BigEndian()) {
		hdr.magic = Sys_SwapUint32(hdr.magic);
		hdr.texSize = Sys_SwapUint32(hdr.texSize);
		hdr.glyphCount = Sys_SwapUint32(hdr.glyphCount);
	}

	if (hdr.magic != FNT_MAGIC)
		return false;

	fnt->glyphCount = hdr.glyphCount;

	fnt->glyphs = Sys_Alloc(fnt->glyphCount, sizeof(*fnt->glyphs), MH_Asset);
	if (E_ReadStream(stm, fnt->glyphs, sizeof(*fnt->glyphs) * fnt->glyphCount) != sizeof(*fnt->glyphs) * fnt->glyphCount)
		goto error;

	if (Sys_BigEndian()) {
		for (uint32_t i = 0; i < fnt->glyphCount; ++i) {
			fnt->glyphs[i].u = Sys_SwapFloat(fnt->glyphs[i].u);
			fnt->glyphs[i].v = Sys_SwapFloat(fnt->glyphs[i].v);
			fnt->glyphs[i].tw = Sys_SwapFloat(fnt->glyphs[i].tw);
			fnt->glyphs[i].th = Sys_SwapFloat(fnt->glyphs[i].th);
			
			fnt->glyphs[i].bearing.x = Sys_SwapInt32(fnt->glyphs[i].bearing.x);
			fnt->glyphs[i].bearing.y = Sys_SwapInt32(fnt->glyphs[i].bearing.y);
			fnt->glyphs[i].size.w = Sys_SwapInt32(fnt->glyphs[i].size.w);
			fnt->glyphs[i].size.h = Sys_SwapInt32(fnt->glyphs[i].size.h);
			fnt->glyphs[i].adv = Sys_SwapInt32(fnt->glyphs[i].adv);
		}
	}
	
	texSize = Re_deviceInfo.limits.maxTextureSize;
	if (texSize < hdr.texSize) {
		uint32_t curSize = hdr.texSize;

		while (curSize > texSize) {
			E_StreamSeek(stm, curSize * curSize, IO_SEEK_CUR);
			curSize /= 2;
		}
	} else {
		texSize = hdr.texSize;
	}

	texDataSize = texSize * texSize;
	texData = Sys_Alloc(texDataSize, 1, MH_Asset);
	if (E_ReadStream(stm, texData, texDataSize) != texDataSize)
		goto error;

	struct TextureCreateInfo tci =
	{
		.desc.width = texSize,
		.desc.height = texSize,
		.desc.depth = 1,
		.desc.type = TT_2D,
		.desc.usage = TU_SAMPLED | TU_TRANSFER_DST,
		.desc.format = TF_R8_UNORM,
		.desc.arrayLayers = 1,
		.desc.mipLevels = 1,
		.desc.gpuOptimalTiling = true,
		.desc.memoryType = MT_GPU_LOCAL,
		.data = texData,
		.dataSize = texDataSize,
		.keepData = false
	};

	fnt->texture = E_CreateResource("__SysFont_Texture", RES_TEXTURE, &tci);
	if (fnt->texture == E_INVALID_HANDLE)
		goto error;

	return true;

error:
	Sys_Free(texData);
	Sys_Free(fnt->glyphs);

	memset(fnt, 0x0, sizeof(*fnt));

	return false;
}
