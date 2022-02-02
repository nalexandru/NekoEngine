#ifndef _NE_SCENE_SCENE_H_
#define _NE_SCENE_SCENE_H_

#include <Engine/Types.h>
#include <Scene/Scene.h>
#include <Runtime/Array.h>
#include <Render/Types.h>
#include <Render/Systems.h>
#include <System/AtomicLock.h>

#define RES_SCENE	"Scene"

struct NeScene
{
	struct NeArray entities, compData, compHandle;
	struct NeCollectDrawablesArgs collect;
	NeBufferHandle sceneData;
	uint32_t maxLights, maxInstances;
	size_t sceneDataSize, lightDataSize, instanceDataSize;
	struct NeAtomicLock compLock;

	uint8_t *dataPtr;
	bool dataTransfered;

	NeHandle environmentMap;
	bool loaded;

	char name[64];
	char path[256];
	uint8_t id;
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

bool Scn_ActivateScene(struct NeScene *scn);
void Scn_DataAddress(const struct NeScene *s, uint64_t *sceneAddress, uint64_t *instanceAddress);

void Scn_StartDrawableCollection(struct NeScene *s, const struct NeCamera *c);
void Scn_StartDataUpdate(struct NeScene *s, const struct NeCamera *c);

bool Scn_CreateTerrain(struct NeScene *scn, const struct NeTerrainCreateInfo *tci);

#endif /* _NE_SCENE_SCENE_H_ */
