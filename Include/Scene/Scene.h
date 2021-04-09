#ifndef _SCN_SCENE_H_
#define _SCN_SCENE_H_

#include <Engine/Types.h>
#include <Scene/Scene.h>
#include <Runtime/Array.h>
#include <Render/Types.h>

#define RES_SCENE	"Scene"

struct Scene
{
	struct Array entities, compData, compHandle;
	struct DescriptorSet *descriptorSet;
	BufferHandle sceneData;

	Handle environmentMap;
	bool loaded;

	wchar_t name[64];
	char path[256];
};

ENGINE_API extern struct Scene *Scn_activeScene;

struct Scene *Scn_CreateScene(const wchar_t *name);
struct Scene *Scn_StartSceneLoad(const char *path);
void Scn_UnloadScene(struct Scene *scn);

bool Scn_ActivateScene(struct Scene *scn);

#endif /* _SCN_SCENE_H_ */
