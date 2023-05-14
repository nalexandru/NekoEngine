#ifndef NE_SCENE_SCENE_H
#define NE_SCENE_SCENE_H

#include <Engine/Types.h>
#include <Scene/Scene.h>
#include <Runtime/Array.h>
#include <Render/Types.h>
#include <Render/Systems.h>
#include <System/AtomicLock.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RES_SCENE	"Scene"

struct NeScene
{
	struct NeArray entities, compData, compFree;
	struct NeCollectDrawablesArgs collect;
	NeBufferHandle sceneData;
	uint32_t maxLights, maxInstances, lightCount;
	size_t sceneDataSize, lightDataSize, instanceDataSize;
	NeHandle camera;

	struct {
		struct NeAtomicLock comp, newComp, entity, newEntity;
	} lock;

	uint8_t *dataPtr;
	bool dataTransferred;

	NeHandle environmentMap;
	bool loaded;

	char name[64];
	char path[256];
	char postLoad[256];
	uint8_t id;

	struct NeArray newEntities, newCompData, newCompOffset;
};

struct NeTerrainCreateInfo
{
	uint16_t tileSize;
	uint16_t tileCount;
	float maxHeight;
	NeHandle material;
	char *mapFile;
};

ENGINE_API extern struct NeScene *Scn_activeScene;

struct NeScene *Scn_GetScene(uint8_t id);

struct NeScene *Scn_CreateScene(const char *name);
struct NeScene *Scn_StartSceneLoad(const char *path);

void Scn_UnloadScene(struct NeScene *scn);
void Scn_UnloadScenes(void);

bool Scn_ActivateScene(struct NeScene *scn);
void Scn_DataAddress(const struct NeScene *s, uint64_t *sceneAddress, uint64_t *instanceAddress);

void Scn_StartDrawableCollection(struct NeScene *s, const struct NeCamera *c);
void Scn_StartDataUpdate(struct NeScene *s, const struct NeCamera *c);

void Scn_Commit(struct NeScene *scn);

const struct NeLightData * const Scn_VisibleLights(struct NeScene *scn);

bool Scn_CreateTerrain(struct NeScene *scn, const struct NeTerrainCreateInfo *tci);

uint32_t Scn_LightCount(struct NeScene *scn);

#ifdef __cplusplus
}
#endif

#endif /* NE_SCENE_SCENE_H */

/* NekoEngine
 *
 * Scene.h
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
