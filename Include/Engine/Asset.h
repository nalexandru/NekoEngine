#ifndef _NE_ENGINE_ASSET_H_
#define _NE_ENGINE_ASSET_H_

#include <Engine/Types.h>
#include <Render/Types.h>

struct jsmntok;

struct NeMetadata
{
	char *json;
	struct jsmntok *tokens;
	uint32_t tokenCount;
	int64_t jsonSize;

	uint32_t version;
	const char *id;
};

// Metadata
bool E_LoadMetadata(struct NeMetadata *meta, const char *file);
bool E_LoadMetadataFromFile(struct NeMetadata *meta, NeFile f);
bool E_LoadMetadataFromStream(struct NeMetadata *meta, struct NeStream *stm);
void E_LoadMetadataFloatVector(struct NeMetadata *meta, struct jsmntok *tok, float *out, uint32_t count);

// Models
bool E_LoadNMeshAsset(struct NeStream *stm, struct NeModel *m);

// Animation
bool E_LoadNAnimAsset(struct NeStream *stm, struct NeAnimationClip *ac);

// Textures
bool E_LoadImageAssetComp(struct NeStream *stm, struct NeTextureCreateInfo *tex, bool flip, int rcomp);
static inline bool E_LoadImageAsset(struct NeStream *stm, struct NeTextureCreateInfo *tex, bool flip) { return E_LoadImageAssetComp(stm, tex, flip, 4); }
bool E_LoadHDRAsset(struct NeStream *stm, struct NeTextureCreateInfo *tci, bool flip);
bool E_LoadTGAAsset(struct NeStream *stm, struct NeTextureCreateInfo *tex);
bool E_LoadDDSAsset(struct NeStream *stm, struct NeTextureCreateInfo *tex);

bool E_LoadOggAsset(struct NeStream *stm, struct NeAudioClip *clip);
bool E_LoadWaveAsset(struct NeStream *stm, struct NeAudioClip *clip);

bool E_LoadFontAsset(struct NeStream *stm, struct NeFont *font);

#endif /* _NE_ENGINE_ASSET_H_ */
