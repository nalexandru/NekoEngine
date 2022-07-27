#include <stdio.h>
#include <ctype.h>

#include <Math/Math.h>
#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Engine/Config.h>
#include <System/Log.h>
#include <Engine/Events.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Engine/Profiler.h>
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
#define SCNMOD				"Scene"
#define BUFF_SZ				512
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

#pragma pack(push, 1)
struct NeSceneData
{
	struct {
		struct NeMatrix viewProjection;
		struct NeMatrix inverseProjection;
		struct NeVec4 cameraPosition;
	} NE_ALIGN(16) camera;

	struct {
		struct NeVec4 sunPosition;
		uint32_t enviornmentMap;
		uint32_t irradianceMap;
		uint32_t aoMap;
		uint32_t __padding;
	} NE_ALIGN(16) enviornment;
	
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
} NE_ALIGN(16);
#pragma pack(pop)

struct NeScene *Scn_activeScene = NULL;

static uint8_t _nextSceneId = 0;
static struct NeScene *_scenes[UINT8_MAX] = { 0 };

static inline bool _InitScene(struct NeScene *s);
static void _LoadJob(int worker, struct NeScene *scn);
static inline void _ReadSceneInfo(struct NeScene *s, struct NeStream *stm, char *data);
static inline void _ReadTerrain(struct NeScene *s, struct NeStream *stm, char *data);
static inline void _ReadEntity(struct NeScene *s, char *name, struct NeStream *stm, char *data, struct NeArray *args);
static inline uint64_t _DataOffset(const struct NeScene *s);
static int32_t _SortDrawables(const struct NeDrawable *a, const struct NeDrawable *b);

struct NeScene *
Scn_GetScene(uint8_t id)
{
	return _scenes[id];
}

struct NeScene *
Scn_CreateScene(const char *name)
{
	struct NeScene *s = Sys_Alloc(sizeof(struct NeScene), 1, MH_Scene);
	if (!s)
		return NULL;

	s->maxLights = DEF_MAX_LIGHTS;
	s->maxInstances = DEF_MAX_INSTANCES;

	snprintf(s->name, sizeof(s->name), "%s", name);

	if (!_InitScene(s)) {
		Sys_Free(s);
		return NULL;
	}

	return s;
}

struct NeScene *
Scn_StartSceneLoad(const char *path)
{
	struct NeScene *s = Sys_Alloc(sizeof(struct NeScene), 1, MH_Scene);
	if (!s)
		return NULL;

	snprintf(s->path, sizeof(s->path), "%s", path);

	if (E_GetCVarBln("Engine_SingleThreadSceneLoad", false))
		_LoadJob(0, s);
	else
		E_ExecuteJob((NeJobProc)_LoadJob, s, NULL, NULL);

	return s;
}

void
Scn_UnloadScene(struct NeScene *s)
{
	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		Rt_TermArray(&s->collect.instanceArrays[i]);
		Rt_TermArray(&s->collect.opaqueDrawableArrays[i]);
		Rt_TermArray(&s->collect.blendedDrawableArrays[i]);
	}
	Sys_Free(s->collect.instanceOffset);
	Sys_Free(s->collect.instanceArrays);
	Sys_Free(s->collect.opaqueDrawableArrays);
	Sys_Free(s->collect.blendedDrawableArrays);

	Rt_TermArray(&s->collect.blendedDrawables);

	if (s->environmentMap != E_INVALID_HANDLE)
		E_UnloadResource(s->environmentMap);

	Re_Destroy(s->sceneData);

	E_TermSceneEntities(s);
	E_TermSceneComponents(s);

	Sys_Free(s);
}

bool
Scn_ActivateScene(struct NeScene *s)
{
	if (!s->loaded)
		return false;

	Scn_activeScene = s;
	E_Broadcast(EVT_SCENE_ACTIVATED, s);

	return true;
}

void
Scn_DataAddress(const struct NeScene *s, uint64_t *sceneAddress, uint64_t *instanceAddress)
{
	const uint64_t offset = _DataOffset(s);
	*sceneAddress = Re_BufferAddress(s->sceneData, offset);
	*instanceAddress = Re_BufferAddress(s->sceneData, offset + sizeof(struct NeSceneData) + s->lightDataSize);
}

void
Scn_StartDrawableCollection(struct NeScene *s, const struct NeCamera *c)
{
	Prof_InsertMarker("BeginDrawableCollection");

	s->collect.nextArray = 0;
	M_MulMatrix(&s->collect.vp, &c->projMatrix, &c->viewMatrix);

	M_FrustumFromVP(&s->collect.camFrustum, &c->viewMatrix, &c->projMatrix);
//	memcpy(&s->collect.camFrustum, &c->frustum, sizeof(s->collect.camFrustum));

	const NeEntityHandle camEnt = c->_owner;
	const struct NeTransform *camXform = E_GetComponentS(s, camEnt, E_ComponentTypeId(TRANSFORM_COMP));
	M_Copy(&s->collect.camPos, &camXform->position);

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		Rt_ClearArray(&s->collect.opaqueDrawableArrays[i], false);
		Rt_ClearArray(&s->collect.blendedDrawableArrays[i], false);
		Rt_ClearArray(&s->collect.instanceArrays[i], false);
	}
	Rt_ClearArray(&s->collect.blendedDrawables, false);

	E_ExecuteSystemS(s, RE_COLLECT_DRAWABLES, &s->collect);

	uint32_t offset = 0;
	uint8_t *dst = s->dataPtr + _DataOffset(s) + sizeof(struct NeSceneData) + s->lightDataSize;
	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		s->collect.instanceOffset[i] = offset;
		offset += (uint32_t)s->collect.instanceArrays[i].count;

		const size_t sz = Rt_ArrayUsedByteSize(&s->collect.instanceArrays[i]);
		if (!sz)
			continue;

		memcpy(dst, s->collect.instanceArrays[i].data, sz);
		dst += sz;

		struct NeDrawable *d = NULL;
		Rt_ArrayForEach(d, &Scn_activeScene->collect.blendedDrawableArrays[i]) {
			d->instanceId += s->collect.instanceOffset[i];
			Rt_ArrayAdd(&s->collect.blendedDrawables, d);
		}
	}

	Rt_ArraySort(&s->collect.blendedDrawables, (RtSortFunc)_SortDrawables);

	Prof_InsertMarker("EndDrawableCollection");
}

void
Scn_StartDataUpdate(struct NeScene *s, const struct NeCamera *c)
{
	Prof_InsertMarker("BeginDataUpdate");

	s->dataTransfered = false;

	struct NeCollectLights collect;
	Rt_InitArray(&collect.lightData, s->maxLights, sizeof(struct NeLightData), MH_Transient);

	E_ExecuteSystemS(s, SCN_COLLECT_LIGHTS, &collect);
	Prof_InsertMarker("CollectLights");

	uint8_t *dst = s->dataPtr + _DataOffset(s);

	const NeEntityHandle camEnt = c->_owner;
	const struct NeTransform *camXform = E_GetComponentS(s, camEnt, E_ComponentTypeId(TRANSFORM_COMP));

	const uint32_t tileSize = E_GetCVarU32(SID("Render_LightCullingTileSize"), 16)->u32;

	struct NeSceneData data =
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
			.xTileCount = (*E_screenWidth + (*E_screenWidth % tileSize)) / tileSize
		},
		.settings =
		{
			.exposure = 1.f,
			.gamma = 2.2f,
			.sampleCount = 1
		}
	};
	M_MulMatrix(&data.camera.viewProjection, &c->projMatrix, &c->viewMatrix);
	M_InverseMatrix(&data.camera.inverseProjection, &c->projMatrix);

	data.settings.invGamma = 1.f / data.settings.gamma;

	memcpy(dst, &data, sizeof(data));
	memcpy(dst + sizeof(data), collect.lightData.data, Rt_ArrayUsedByteSize(&collect.lightData));

	s->lightCount = data.lighting.lightCount;
	s->dataTransfered = true;

	Prof_InsertMarker("EndDataUpdate");
}

const struct NeLightData * const
Scn_VisibleLights(struct NeScene *scn)
{
	return (const struct NeLightData * const)(scn->dataPtr + _DataOffset(scn) + sizeof(struct NeSceneData));
}

bool
_InitScene(struct NeScene *s)
{
	Sys_InitAtomicLock(&s->compLock);

	if (!E_InitSceneComponents(s) || !E_InitSceneEntities(s))
		goto error;

	s->lightDataSize = ROUND_UP(sizeof(struct NeLightData) * s->maxLights, 16);
	s->instanceDataSize = ROUND_UP(sizeof(struct NeModelInstance) * s->maxInstances, 16);
	s->sceneDataSize = sizeof(struct NeSceneData) + s->lightDataSize + s->instanceDataSize;

	struct NeBufferCreateInfo bci =
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

	s->collect.opaqueDrawableArrays = Sys_Alloc(E_JobWorkerThreads(), sizeof(struct NeArray), MH_Scene);
	if (!s->collect.opaqueDrawableArrays)
		goto error;

	s->collect.blendedDrawableArrays = Sys_Alloc(E_JobWorkerThreads(), sizeof(struct NeArray), MH_Scene);
	if (!s->collect.blendedDrawableArrays)
		goto error;

	s->collect.instanceArrays = Sys_Alloc(E_JobWorkerThreads(), sizeof(struct NeArray), MH_Scene);
	if (!s->collect.instanceArrays)
		goto error;

	s->collect.instanceOffset = Sys_Alloc(E_JobWorkerThreads(), sizeof(uint32_t), MH_Scene);
	if (!s->collect.instanceOffset)
		goto error;

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		if (!Rt_InitArray(&s->collect.opaqueDrawableArrays[i], 10, sizeof(struct NeDrawable), MH_Scene))
			goto error;

		if (!Rt_InitArray(&s->collect.blendedDrawableArrays[i], 10, sizeof(struct NeDrawable), MH_Scene))
			goto error;

		if (!Rt_InitArray(&s->collect.instanceArrays[i], 10, sizeof(struct NeModelInstance), MH_Scene))
			goto error;
	}

	Rt_InitArray(&s->collect.blendedDrawables, 10, sizeof(struct NeDrawable), MH_Scene);

	s->collect.s = s;

	_scenes[_nextSceneId] = s;
	s->id = _nextSceneId++;

	return true;

error:
	if (s->sceneData)
		Re_Destroy(s->sceneData);

	E_TermSceneEntities(s);
	E_TermSceneComponents(s);

	return false;
}

void
_LoadJob(int wid, struct NeScene *s)
{
	struct NeStream stm;
	char *data = NULL;
	struct NeArray args;

	if (!E_FileStream(s->path, IO_READ, &stm)) {
		Sys_LogEntry(SCNMOD, LOG_CRITICAL, "Failed to open scene file %s", s->path);
		return;
	}

	E_Broadcast(EVT_SCENE_LOAD_STARTED, s);

	data = Sys_Alloc(sizeof(char), BUFF_SZ, MH_Transient);
	Rt_InitPtrArray(&args, 10, MH_Scene);

	while (!E_EndOfStream(&stm)) {
		char *line = E_ReadStreamLine(&stm, data, BUFF_SZ);
		size_t len;

		if (!*(line = Rt_SkipWhitespace(line)) || line[0] == '#')
			continue;

		len = strnlen(line, BUFF_SZ);
		if (!strncmp(line, "SceneInfo", len)) {
			_ReadSceneInfo(s, &stm, data);
		} else if (!strncmp(line, "Terrain", len)) {
			_ReadTerrain(s, &stm, data);
		} else if (!strncmp(line, "EndSceneInfo", len)) {
			//
		} else if (strstr(line, "Entity")) {
			char *name = strchr(line, '=') + 1;
			_ReadEntity(s, name, &stm, data, &args);
		}

		memset(data, 0x0, BUFF_SZ);
	}

	E_CloseStream(&stm);

	E_ExecuteSystemS(s, ANIM_BUILD_SKELETON, NULL);

	s->loaded = true;
	if (s->postLoad[0]) {
		lua_State *vm = Sc_CreateVM();

		lua_pushlightuserdata(vm, s);
		lua_setglobal(vm, "LoadedScene");

		const char *err = Sc_ExecuteFile(vm, s->postLoad);
		if (err)
			Sys_LogEntry(SCNMOD, LOG_CRITICAL, "PostLoad script execution failed for scene %s: %s", s->path, err);

		Sc_DestroyVM(vm);
	}

	E_Broadcast(EVT_SCENE_LOADED, s);

	Rt_TermArray(&args);
}

void
_ReadSceneInfo(struct NeScene *s, struct NeStream *stm, char *data)
{
	s->maxLights = DEF_MAX_LIGHTS;
	s->maxInstances = DEF_MAX_INSTANCES;
	
	while (!E_EndOfStream(stm)) {
		char *line = E_ReadStreamLine(stm, data, BUFF_SZ);
		size_t len;

		if (!*(line = Rt_SkipWhitespace(line)) || line[0] == '#')
			continue;

		len = strnlen(line, BUFF_SZ);
		if (!strncmp(line, "Name", 4)) {
			char *type = strchr(line, '=') + 1;
			snprintf(s->name, sizeof(s->name), "%s", type);
		} else if (!strncmp(line, "EnvironmentMap", 14)) {
			char *file = strchr(line, '=') + 1;
			s->environmentMap = E_LoadResource(file, RES_TEXTURE);
		} else if (!strncmp(line, "MaxLights", 9)) {
			s->maxLights = atoi(strchr(line, '=') + 1);
		} else if (!strncmp(line, "PostLoad", 8)) {
			char *postLoad = strchr(line, '=') + 1;
			snprintf(s->postLoad, sizeof(s->postLoad), "%s", postLoad);
		} else if (!strncmp(line, "EndSceneInfo", len)) {
			break;
		}
	}

	_InitScene(s);
}

void
_ReadTerrain(struct NeScene *s, struct NeStream *stm, char *data)
{
	struct NeTerrainCreateInfo tci = { 0 };
	
	while (!E_EndOfStream(stm)) {
		char *line = E_ReadStreamLine(stm, data, BUFF_SZ);
		size_t len;

		if (!*(line = Rt_SkipWhitespace(line)) || line[0] == '#')
			continue;
		
		len = strnlen(line, BUFF_SZ);

		if (!strncmp(line, "TileSize", 8)) {
			tci.tileSize = atoi(strchr(line, '=') + 1);
		} else if (!strncmp(line, "TileCount", 9)) {
			tci.tileCount = atoi(strchr(line, '=') + 1);
		} else if (!strncmp(line, "Material", 8)) {
			tci.material = E_LoadResource(strchr(line, '=') + 1, RES_MATERIAL);
		} else if (!strncmp(line, "Map", 3)) {
			tci.mapFile = Rt_StrDup(strchr(line, '=') + 1, MH_Asset);
		} else if (!strncmp(line, "MaxHeight", 9)) {
			tci.maxHeight = (float)atof(strchr(line, '=') + 1);
		} else if (!strncmp(line, "EndTerrain", len)) {
			break;
		}
	}
	
	Scn_CreateTerrain(s, &tci);
}

void
_ReadEntity(struct NeScene *s, char *name, struct NeStream *stm, char *data, struct NeArray *args)
{
	NeEntityHandle entity = NULL;
	char *compType = NULL;
	char entityName[MAX_ENTITY_NAME];

	snprintf(entityName, sizeof(entityName), "%s", name);
	compType = Sys_Alloc(sizeof(*compType), BUFF_SZ, MH_Transient);

	while (!E_EndOfStream(stm)) {
		char *line = E_ReadStreamLine(stm, data, BUFF_SZ);
		size_t len;

		if (!*(line = Rt_SkipWhitespace(line)) || line[0] == '#')
			continue;

		len = strnlen(line, BUFF_SZ);

		if (!strncmp(line, "Component=", 10)) {
			char *type = strchr(line, '=') + 1;
			if (!type)
				continue;

			strncpy(compType, type, BUFF_SZ);

			if (!entity)
				entity = E_CreateEntityS(s, entityName, NULL);

			Rt_ZeroArray(args);
		} else if (!strncmp(line, "EndComponent", len)) {
			if (!entity) {
				Sys_LogEntry(SCNMOD, LOG_WARNING, "Component declared outside entity");
				continue;
			}

			void *guard = NULL;
			Rt_ArrayAddPtr(args, guard);

			E_AddNewComponentS(s, entity, E_ComponentTypeId(compType), (const void **)args->data);
			memset(compType, 0x0, BUFF_SZ);
		} else if (!strncmp(line, "EndEntity", len)) {
			break;
		} else if (compType[0] == 0x0) {
			if (!strncmp(line, "Type=", 5)) {
				char *type = strchr(line, '=') + 1;
				if (!type)
					continue;

				entity = E_CreateEntityS(s, entityName, type);
			}
		} else {
			char *arg = line;
			char *val = strchr(line, '=');
			if (!val)
				continue;

			*val++ = 0x0;

			Rt_ArrayAddPtr(args, Rt_StrNDup(arg, BUFF_SZ, MH_Transient));
			Rt_ArrayAddPtr(args, Rt_StrNDup(val, BUFF_SZ, MH_Transient));
		}
	}
}

static inline uint64_t
_DataOffset(const struct NeScene *s)
{
	return s->sceneDataSize * Re_frameId;
}

static int32_t
_SortDrawables(const struct NeDrawable *a, const struct NeDrawable *b)
{
	if (a->distance < b->distance)
		return 1;
	else if (a->distance > b->distance)
		return -1;
	else
		return 0;
}
