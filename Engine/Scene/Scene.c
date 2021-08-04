#include <stdio.h>
#include <ctype.h>

#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Engine/Config.h>
#include <System/Log.h>
#include <Engine/Events.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Scene/Scene.h>
#include <Scene/Light.h>
#include <Scene/Camera.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Engine/Resource.h>
#include <Render/Render.h>
#include <Render/Model.h>
#include <Animation/Animation.h>

#include "../Engine/ECS.h"

#define DEF_MAX_LIGHTS		4096
#define DEF_MAX_INSTANCES	8192
#define SCNMOD			L"Scene"
#define BUFF_SZ			512
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

#pragma pack(push, 1)
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
	} lighting;

	uint64_t instanceBufferAddress;

	struct {
		float exposure;
		float gamma;
		float invGamma;
		uint32_t sampleCount;
	} settings;
};
#pragma pack(pop)

struct Scene *Scn_activeScene = NULL;

static inline bool _InitScene(struct Scene *s);
static void _LoadJob(int worker, struct Scene *scn);
static inline void _ReadSceneInfo(struct Scene *s, struct Stream *stm, char *data, wchar_t *buff);
static inline void _ReadEntity(struct Scene *s, struct Stream *stm, char *data, wchar_t *wbuff, struct Array *args);
static inline uint64_t _DataOffset(const struct Scene *s);

struct Scene *
Scn_CreateScene(const wchar_t *name)
{
	struct Scene *s = Sys_Alloc(sizeof(struct Scene), 1, MH_Scene);
	if (!s)
		return NULL;

	s->maxLights = DEF_MAX_LIGHTS;
	s->maxInstances = DEF_MAX_INSTANCES;

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

	if (E_GetCVarBln(L"Engine_SingleThreadSceneLoad", false))
		_LoadJob(0, s);
	else
		E_ExecuteJob((JobProc)_LoadJob, s, NULL);

	return s;
}

void
Scn_UnloadScene(struct Scene *s)
{
	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		Rt_TermArray(&s->collect.instanceArrays[i]);
		Rt_TermArray(&s->collect.arrays[i]);
	}
	Sys_Free(s->collect.instanceArrays);
	Sys_Free(s->collect.arrays);

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

void
Scn_DataAddress(const struct Scene *s, uint64_t *sceneAddress, uint64_t *instanceAddress)
{
	const uint64_t offset = _DataOffset(s);
	*sceneAddress = Re_BufferAddress(s->sceneData, offset);
	*instanceAddress = Re_BufferAddress(s->sceneData, offset + sizeof(struct SceneData) + s->lightDataSize);
}

void
Scn_StartDrawableCollection(struct Scene *s, const struct Camera *c)
{
	s->collect.nextArray = 0;
	m4_mul(&s->collect.vp, &c->projMatrix, &c->viewMatrix);

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		Rt_ClearArray(&s->collect.arrays[i], false);
		Rt_ClearArray(&s->collect.instanceArrays[i], false);
	}

	E_ExecuteSystemS(s, RE_COLLECT_DRAWABLES, &s->collect);

	uint8_t *dst = s->dataPtr + _DataOffset(s) + sizeof(struct SceneData) + s->lightDataSize;
	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		const size_t sz = Rt_ArrayUsedByteSize(&s->collect.instanceArrays[i]);
		if (!sz)
			continue;

		memcpy(dst, s->collect.instanceArrays[i].data, sz);
		dst += sz;
	}
}

void
Scn_StartDataUpdate(struct Scene *s, const struct Camera *c)
{
	s->dataTransfered = false;

	struct CollectLights collect;
	Rt_InitArray(&collect.lightData, s->maxLights, sizeof(struct LightData), MH_Transient);

	E_ExecuteSystemS(s, SCN_COLLECT_LIGHTS, &collect);

	uint8_t *dst = s->dataPtr + _DataOffset(s);

	const EntityHandle camEnt = c->_owner;
	const struct Transform *camXform = E_GetComponentS(s, camEnt, E_ComponentTypeId(TRANSFORM_COMP));

	struct SceneData data =
	{
		.camera =
		{
			.cameraPosition = { -camXform->position.x, camXform->position.y, -camXform->position.z, 0.f }
		},
		.enviornment =
		{
			.sunPosition = { 0.f, 1.f, 1.f, 1.f },
			.enviornmentMap = E_ResHandleToGPU(s->environmentMap)
		},
		.lighting =
		{
			.lightCount = (uint32_t)collect.lightData.count,
			.xTileCount = *E_screenWidth + (*E_screenWidth % 16) / 16
		},
		.settings =
		{
			.exposure = 1.f,
			.gamma = 2.2f,
			.sampleCount = 1
		}
	};
	m4_mul(&data.camera.viewProjection, &c->projMatrix, &c->viewMatrix);

	data.settings.invGamma = 1.f / data.settings.gamma;

	memcpy(dst, &data, sizeof(data));
	memcpy(dst + sizeof(data), collect.lightData.data, Rt_ArrayUsedByteSize(&collect.lightData));

	s->dataTransfered = true;
}

bool
_InitScene(struct Scene *s)
{
	if (!E_InitSceneComponents(s) || !E_InitSceneEntities(s))
		goto error;

	s->lightDataSize = ROUND_UP(sizeof(struct LightData) * s->maxLights, 16);
	s->instanceDataSize = ROUND_UP(sizeof(struct ModelInstance) * s->maxInstances, 16);
	s->sceneDataSize = sizeof(struct SceneData) + s->lightDataSize + s->instanceDataSize;

	struct BufferCreateInfo bci =
	{
		.desc =
		{
			.size = s->sceneDataSize * RE_NUM_FRAMES,
			.usage = BU_TRANSFER_DST | BU_STORAGE_BUFFER,
			.memoryType = MT_CPU_COHERENT
		}
	};

	if (!Re_CreateBuffer(&bci, &s->sceneData))
		goto error;

	s->dataPtr = Re_MapBuffer(s->sceneData);
	if (!s->dataPtr)
		goto error;

	s->collect.arrays = Sys_Alloc(E_JobWorkerThreads(), sizeof(struct Array), MH_Scene);
	if (!s->collect.arrays)
		goto error;

	s->collect.instanceArrays = Sys_Alloc(E_JobWorkerThreads(), sizeof(struct Array), MH_Scene);
	if (!s->collect.instanceArrays)
		goto error;

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		if (!Rt_InitArray(&s->collect.arrays[i], 10, sizeof(struct Drawable), MH_Scene))
			goto error;

		if (!Rt_InitArray(&s->collect.instanceArrays[i], 10, sizeof(struct ModelInstance), MH_Scene))
			goto error;
	}

	s->collect.s = s;

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
	s->maxLights = DEF_MAX_LIGHTS;
	s->maxInstances = DEF_MAX_INSTANCES;
	
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
		} else if (!strncmp(line, "MaxLights", 9)) {
			s->maxLights = atoi(strchr(line, '=') + 1);
		} else if (!strncmp(line, "EndSceneInfo", len)) {
			break;
		}
	}

	_InitScene(s);
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

static inline uint64_t
_DataOffset(const struct Scene *s)
{
	return s->sceneDataSize * Re_frameId;
}

