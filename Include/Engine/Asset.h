#ifndef _E_ASSET_H_
#define _E_ASSET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Engine/Types.h>

// Models
bool E_LoadNMeshAsset(struct Stream *stm, struct Model *m);
bool E_LoadglTFAsset(const char *baseDir, struct Stream *stm, struct Model *m);

// Textures
bool E_LoadImageAsset(struct Stream *stm, struct TextureCreateInfo *tex);
bool E_LoadTGAAsset(struct Stream *stm, struct TextureCreateInfo *tex);

bool E_LoadOggAsset(struct Stream *stm, struct AudioClip *clip);
bool E_LoadWaveAsset(struct Stream *stm, struct AudioClip *clip);

bool E_LoadFontAsset(struct Stream *stm, struct Font *font);

#ifdef __cplusplus
}
#endif

#endif /* _E_ASSET_H_ */
