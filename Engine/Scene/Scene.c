#include <stdio.h>
#include <ctype.h>

#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Engine/Config.h>
#include <System/Log.h>
#include <Engine/Events.h>
#include <Engine/ECSystem.h>
#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Engine/Resource.h>
#include <Render/Render.h>
#include <Animation/Animation.h>

#include "../Engine/ECS.h"

#define SCNMOD	L"Scene"
#define BUFF_SZ	512

struct SceneData
{
	struct {
		struct mat4 viewProjection;
		struct vec4 cameraPosition;
	} camera;

	struct {
		struct vec4 sunPosition;
		uint32_t enviornmentMap;
		uint32_t __padding[3];
	} enviornment;

	struct {
		uint32_t lightCount;
		uint32_t xTileCount;
		uint32_t __padding[2];
	} lighting;

	struct {
		float exposure;
		float gamma;
		uint32_t sampleCount;
		uint32_t __padding[1];
	} settings;
};

struct Scene *Scn_activeScene = NULL;

static inline bool _InitScene(struct Scene *s);
static void _LoadJob(int worker, struct Scene *scn);
static inline void _ReadSceneInfo(struct Scene *s, struct Stream *stm, char *data, wchar_t *buff);
static inline void _ReadEntity(struct Scene *s, struct Stream *stm, char *data, wchar_t *wbuff, struct Array *args);

struct Scene *
Scn_CreateScene(const wchar_t *name)
{
	struct Scene *s = Sys_Alloc(sizeof(struct Scene), 1, MH_Scene);
	if (!s)
		return NULL;

	swprintf(s->name, sizeof(s->name) / sizeof(wchar_t), L"%ls", name);

	if (!_InitScene(s)) {
		Sys_Free(s);
		return NULL;
	}

	return s;
}

struct Scene *
Scn_StartSceneLoad(const char *path)
{
	struct Scene *s = Sys_Alloc(sizeof(struct Scene), 1, MH_Scene);
	if (!s)
		return NULL;

	snprintf(s->path, sizeof(s->path), "%s", path);

	if (!_InitScene(s)) {
		Sys_Free(s);
		return NULL;
	}

	if (E_GetCVarBln(L"Engine_SingleThreadSceneLoad", false))
		_LoadJob(0, s);
	else
		E_ExecuteJob((JobProc)_LoadJob, s, NULL);

	return s;
}

void
Scn_UnloadScene(struct Scene *s)
{
	Re_Destroy(s->sceneData);

	E_TermSceneEntities(s);
	E_TermSceneComponents(s);

	Sys_Free(s);
}

bool
Scn_ActivateScene(struct Scene *s)
{
	if (!s->loaded)
		return false;

	Scn_activeScene = s;

	return true;
}

bool
_InitScene(struct Scene *s)
{
	if (!E_InitSceneComponents(s) || !E_InitSceneEntities(s))
		goto error;

	struct BufferCreateInfo bci =
	{
		.desc =
		{
			.size = sizeof(struct SceneData) * RE_NUM_FRAMES,
			.usage = BU_TRANSFER_DST | BU_UNIFORM_BUFFER,
			.memoryType = MT_GPU_LOCAL
		}
	};

	if (!Re_CreateBuffer(&bci, &s->sceneData))
		goto error;

	return true;

error:
	if (s->sceneData)
		Re_Destroy(s->sceneData);

	E_TermSceneEntities(s);
	E_TermSceneComponents(s);

	return false;
}

void
_LoadJob(int wid, struct Scene *s)
{
	struct Stream stm;
	char *data = NULL;
	wchar_t *wbuff = NULL;
	struct Array args;

	if (!E_FileStream(s->path, IO_READ, &stm)) {
		Sys_LogEntry(SCNMOD, LOG_CRITICAL, L"Failed to open scene file %hs", s->path);
		return;
	}

	E_Broadcast(EVT_SCENE_LOAD_STARTED, s);

	data = Sys_Alloc(sizeof(char), BUFF_SZ, MH_Transient);
	wbuff = Sys_Alloc(sizeof(wchar_t), BUFF_SZ, MH_Transient);

	Rt_InitPtrArray(&args, 10, MH_Scene);

	while (!E_EndOfStream(&stm)) {
		char *line = E_ReadStreamLine(&stm, data, BUFF_SZ);
		size_t len;

		if (!*(line = Rt_SkipWhitespace(line)) || line[0] == '#')
			continue;

		len = strlen(line);

		if (!strncmp(line, "SceneInfo", len)) {
			_ReadSceneInfo(s, &stm, data, wbuff);
		} else if (!strncmp(line, "EndSceneInfo", len)) {
			//
		} else if (strstr(line, "Entity")) {
			_ReadEntity(s, &stm, data, wbuff, &args);
		}

		memset(data, 0x0, BUFF_SZ);
	}

	E_CloseStream(&stm);

	E_ExecuteSystemS(s, ANIM_BUILD_SKELETON, NULL);

	s->loaded = true;
	E_Broadcast(EVT_SCENE_LOADED, s);

	Rt_TermArray(&args);
}

void
_ReadSceneInfo(struct Scene *s, struct Stream *stm, char *data, wchar_t *buff)
{
	while (!E_EndOfStream(stm)) {
		char *line = E_ReadStreamLine(stm, data, BUFF_SZ);
		size_t len;

		if (!*(line = Rt_SkipWhitespace(line)) || line[0] == '#')
			continue;

		len = strlen(line);

		memset(buff, 0x0, BUFF_SZ * sizeof(*buff));
		if (!strncmp(line, "Name", 4)) {
			char *type = strchr(line, '=') + 1;
			mbstowcs(s->name, type, sizeof(s->name) / sizeof(wchar_t));
		} else if (!strncmp(line, "EnvironmentMap", 14)) {
//			char *file = strchr(line, '=') + 1;
//			s->environmentMap = E_LoadResource(file, RES_TEXTURE);
		} else if (!strncmp(line, "EndSceneInfo", len)) {
			break;
		}
	}
}

void
_ReadEntity(struct Scene *s, struct Stream *stm, char *data, wchar_t *wbuff, struct Array *args)
{
	EntityHandle entity = NULL;
	wchar_t *compType = NULL;

	compType = Sys_Alloc(sizeof(wchar_t), BUFF_SZ, MH_Transient);

	while (!E_EndOfStream(stm)) {
		char *line = E_ReadStreamLine(stm, data, BUFF_SZ);
		size_t len;

		if (!*(line = Rt_SkipWhitespace(line)) || line[0] == '#')
			continue;

		len = strlen(line);

		if (!strncmp(line, "Component=", 10)) {
			char *type = strchr(line, '=') + 1;
			mbstowcs(compType, type, BUFF_SZ);

			if (!entity)
				entity = E_CreateEntityS(s, NULL);

			Rt_ZeroArray(args);
		} else if (!strncmp(line, "EndComponent", len)) {
			if (!entity) {
				Sys_LogEntry(SCNMOD, LOG_WARNING, L"Component declared outside entity");
				continue;
			}

			E_AddNewComponentS(s, entity, E_ComponentTypeId(compType), (const void **)args->data);
			memset(compType, 0x0, sizeof(wchar_t) * BUFF_SZ);
		} else if (!strncmp(line, "EndEntity", len)) {
			break;
		} else if (compType[0] == 0x0) {
			if (!strncmp(line, "Type=", 5)) {
				mbstowcs(wbuff, line, BUFF_SZ);
				E_CreateEntityS(s, wbuff);
			}
		} else {
			char *arg = line, *dst = NULL;
			char *val = strchr(line, '=');
			if (!val)
				continue;

			*val++ = 0x0;

			len = strlen(arg) + 1;
			dst = Sys_Alloc(sizeof(char), len, MH_Transient);
			strncpy(dst, arg, len);
			Rt_ArrayAddPtr(args, dst);

			len = strlen(val) + 1;
			dst = Sys_Alloc(sizeof(char), len, MH_Transient);
			strncpy(dst, val, len);
			Rt_ArrayAddPtr(args, dst);
		}
	}
}
