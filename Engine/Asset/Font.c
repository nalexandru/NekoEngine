#include <stdlib.h>

#include <UI/Font.h>
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Render/Texture.h>

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
	struct TextureCreateInfo tci;
	uint32_t texSize;

	if (E_ReadStream(stm, &hdr, sizeof(hdr)) != sizeof(hdr))
		return false;

	if (hdr.magic != FNT_MAGIC)
		return false;

	fnt->glyphCount = hdr.glyphCount;

	fnt->glyphs = calloc(fnt->glyphCount, sizeof(*fnt->glyphs));
	if (E_ReadStream(stm, fnt->glyphs, sizeof(*fnt->glyphs) * fnt->glyphCount) != sizeof(*fnt->glyphs) * fnt->glyphCount)
		goto error;

	texDataSize = hdr.texSize * hdr.texSize;
	texData = malloc(texDataSize);
	if (E_ReadStream(stm, texData, texDataSize) != texDataSize)
		goto error;

	texSize = hdr.texSize;

	tci.width = texSize;
	tci.height = texSize,
	tci.depth = 1,
	tci.type = TT_2D,
	tci.format = TF_R8_UNORM,
	tci.data = texData,
	tci.dataSize = texDataSize,
	tci.keepData = false;

	fnt->texture = E_CreateResource("__SysFont_Texture", RES_TEXTURE, &tci);
	if (fnt->texture == E_INVALID_HANDLE)
		goto error;

	return true;

error:
	free(texData);
	free(fnt->glyphs);

	return false;
}
