#ifndef _SCN_SCENE_H_
#define _SCN_SCENE_H_

#include <Engine/Types.h>
#include <Scene/Scene.h>
#include <Runtime/Array.h>

#define RES_SCENE	"Scene"

#ifdef __cplusplus
extern "C" {
#endif

struct Scene
{
	wchar_t name[64];
	char path[256];
	bool loaded;
	Array entities, compData, compHandle;
	uint8_t renderDataStart;
};

extern struct Scene *Scn_ActiveScene;
extern struct Scene *Scn_LoadingScene;

struct Scene *Scn_CreateScene(const wchar_t *name);
struct Scene *Scn_StartSceneLoad(const char *path);
void Scn_UnloadScene(struct Scene *scn);

bool Scn_ActivateScene(struct Scene *scn);

#ifdef __cplusplus
}
#endif

#endif /* _SCN_SCENE_H_ */
