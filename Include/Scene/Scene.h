#ifndef _NE_SCENE_SCENE_H_
#define _NE_SCENE_SCENE_H_

#include <Engine/Types.h>
#include <Scene/Scene.h>
#include <Runtime/Array.h>
#include <Render/Types.h>
#include <Render/Systems.h>

#define RES_SCENE	"Scene"

struct Scene
{
	struct Array entities, compData, compHandle;
	struct CollectDrawablesArgs collect;
	BufferHandle sceneData;
	uint32_t maxLights, maxInstances;
	size_t sceneDataSize, lightDataSize, instanceDataSize;

	uint8_t *dataPtr;
	bool dataTransfered;

	Handle environmentMap;
	bool loaded;

	wchar_t name[64];
	char path[256];
};

struct TerrainCreateInfo
{
	uint16_t tileSize;
	uint16_t tileCount;
	float maxHeight;
	Handle material;
	char *mapFile;
};

ENGINE_API extern struct Scene *Scn_activeScene;

struct Scene *Scn_CreateScene(const wchar_t *name);
struct Scene *Scn_StartSceneLoad(const char *path);
void Scn_UnloadScene(struct Scene *scn);

bool Scn_ActivateScene(struct Scene *scn);
void Scn_DataAddress(const struct Scene *s, uint64_t *sceneAddress, uint64_t *instanceAddress);

void Scn_StartDrawableCollection(struct Scene *s, const struct Camera *c);
void Scn_StartDataUpdate(struct Scene *s, const struct Camera *c);

bool Scn_CreateTerrain(struct Scene *scn, const struct TerrainCreateInfo *tci);

#endif /* _NE_SCENE_SCENE_H_ */
