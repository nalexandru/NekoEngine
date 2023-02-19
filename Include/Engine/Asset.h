#ifndef _NE_ENGINE_ASSET_H_
#define _NE_ENGINE_ASSET_H_

#include <Engine/Types.h>
#include <Render/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct jsmntok;

#define ASSET_INFO				\
	union {						\
		uint64_t guard;			\
		struct {				\
			uint32_t id;		\
			uint32_t size;		\
		};						\
	} a = { 0 }

#define ASSET_READ_ID()														\
	if (E_ReadStream(stm, &a.guard, sizeof(a.guard)) != sizeof(a.guard))	\
		goto error 

#define ASSET_CHECK_GUARD(val)												\
	if (E_ReadStream(stm, &a.guard, sizeof(a.guard)) != sizeof(a.guard))	\
		goto error;															\
	if (a.guard != val)														\
		goto error


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
bool E_LoadNMorphAsset(struct NeStream *stm, struct NeModel *m);

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

#ifdef __cplusplus
}
#endif

#endif /* _NE_ENGINE_ASSET_H_ */

/* NekoEngine
 *
 * Asset.h
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
