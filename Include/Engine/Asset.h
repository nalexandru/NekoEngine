#ifndef _NE_ENGINE_ASSET_H_
#define _NE_ENGINE_ASSET_H_

#include <Engine/Types.h>
#include <Render/Types.h>

struct jsmntok;

struct Metadata
{
	char *json;
	struct jsmntok *tokens;
	uint32_t tokenCount;
	int64_t jsonSize;

	uint32_t version;
	const char *id;
};

// Metadata
bool E_LoadMetadata(struct Metadata *meta, const char *file);
bool E_LoadMetadataFromFile(struct Metadata *meta, File f);
bool E_LoadMetadataFromStream(struct Metadata *meta, struct Stream *stm);
void E_LoadMetadataFloatVector(struct Metadata *meta, struct jsmntok *tok, float *out, uint32_t count);

// Models
bool E_LoadNMeshAsset(struct Stream *stm, struct Model *m);

// Animation
bool E_LoadNAnimAsset(struct Stream *stm, struct AnimationClip *ac);

// Textures
bool E_LoadImageAsset(struct Stream *stm, struct TextureCreateInfo *tex);
bool E_LoadTGAAsset(struct Stream *stm, struct TextureCreateInfo *tex);
bool E_LoadDDSAsset(struct Stream *stm, struct TextureCreateInfo *tex);

bool E_LoadOggAsset(struct Stream *stm, struct AudioClip *clip);
bool E_LoadWaveAsset(struct Stream *stm, struct AudioClip *clip);

bool E_LoadFontAsset(struct Stream *stm, struct Font *font);

#endif /* _NE_ENGINE_ASSET_H_ */
