#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Render/Device.h>
#include <Render/Shader.h>
#include <Runtime/Array.h>
#include <Runtime/Json.h>
#include <System/Log.h>

#define SHMOD					L"Shader"

#define SHADER_META_ID			"NeShader"
#define SHADER_META_VER			1

#define SHADER_MODULE_META_ID	"NeShaderModule"
#define SHADER_MODULE_META_VER	1

#define TSTR_GRAPHICS			"graphics"
#define TSTR_COMPUTE			"compute"
#define TSTR_RAY_TRACING		"rayTracing"

#define SSTR_VERTEX				"vertex"
#define SSTR_TESS_CTRL			"tessCtrl"
#define SSTR_TESS_EVAL			"tessEval"
#define SSTR_GEOMETRY			"geometry"
#define SSTR_FRAGMENT			"fragment"
#define SSTR_COMPUTE			"compute"
#define SSTR_RAY_GEN			"rayGen"
#define SSTR_ANY_HIT			"anyHit"
#define SSTR_CLOSEST_HIT		"closestHit"
#define SSTR_MISS				"miss"
#define SSTR_INTERSECTION		"intersection"
#define SSTR_CALLABLE			"callable"
#define SSTR_MESH				"mesh"
#define SSTR_TASK				"task"

struct ShaderModuleInfo
{
	uint64_t hash;
	void *module;
};

static struct Array _modules, _shaders;

static int32_t _sortShaderModules(const struct ShaderModuleInfo *a, const struct ShaderModuleInfo *b);
static int32_t _shaderModuleCompare(const struct ShaderModuleInfo *item, const uint64_t *hash);

static void _loadShader(const char *path);
static int32_t _sortShaders(const struct Shader *a, const struct Shader *b);
static int32_t _shaderCompare(const struct Shader *item, const uint64_t *hash);

bool
Re_LoadShaders(void)
{
	if (!Rt_InitArray(&_shaders, 10, sizeof(struct Shader)))
		return false;
	
	if (!Rt_InitArray(&_modules, 10, sizeof(struct ShaderModuleInfo))) {
		Rt_TermArray(&_shaders);
		return false;
	}
	
	E_ProcessFiles("/Shaders", "shader", true, _loadShader);
	Rt_ArraySort(&_shaders, (RtSortFunc)&_sortShaders);
	
	return true;
}

void
Re_UnloadShaders(void)
{
	Rt_TermArray(&_modules);
	Rt_TermArray(&_shaders);
}

struct Shader *
Re_GetShader(const char *name)
{
	uint64_t hash = Rt_HashString(name);
	return Rt_ArrayBSearch(&_shaders, &hash, (RtCmpFunc)_shaderCompare);
}

static int32_t
_sortShaderModules(const struct ShaderModuleInfo *a, const struct ShaderModuleInfo *b)
{
	if (a->hash == b->hash)
		return 0;
	else if (a->hash > b->hash)
		return -1;
	else
		return 1;
}

static int32_t
_shaderModuleCompare(const struct ShaderModuleInfo *item, const uint64_t *hash)
{
	if (item->hash == *hash)
		return 0;
	else if (item->hash > *hash)
		return -1;
	else
		return 1;
}

static void
_loadShader(const char *path)
{
	struct Metadata meta =
	{
		.version = SHADER_META_VER,
		.id = SHADER_META_ID
	};
	
	if (!E_LoadMetadata(&meta, path))
		return;
	
	struct Shader s = { 0 };

	for (uint32_t i = 0; i < meta.tokenCount; ++i) {
		jsmntok_t key = meta.tokens[i];
		jsmntok_t val = meta.tokens[++i];

		if (JSON_STRING("name", key, meta.json)) {
			memcpy(s.name, meta.json + val.start, val.end - val.start);
		} else if (JSON_STRING("type", key, meta.json)) {
			if (JSON_STRING(TSTR_GRAPHICS, val, meta.json))
				s.type = ST_GRAPHICS;
			else if (JSON_STRING(TSTR_COMPUTE, val, meta.json))
				s.type = ST_COMPUTE;
			else if (JSON_STRING(TSTR_RAY_TRACING, val, meta.json))
				s.type = ST_RAY_TRACING;
		} else if (JSON_STRING("modules", key, meta.json)) {
			if (val.type != JSMN_ARRAY)
				continue;
			
			s.stageCount = val.size;
			s.stages = calloc(s.stageCount, sizeof(*s.stages));
			
			uint32_t pos = i;
			for (uint32_t j = 0; j < val.size; ++j) {
				while (meta.tokens[++pos].type != JSMN_OBJECT) ;
				
				do {
					jsmntok_t mKey = meta.tokens[++pos];
					jsmntok_t mVal = meta.tokens[++pos];
					
					if (JSON_STRING("name", mKey, meta.json)) {
						char *tmp = meta.json + mVal.start;
						meta.json[mVal.end] = 0x0;
						
						uint64_t hash = Rt_HashString(tmp);
						struct ShaderModuleInfo *modInfo = Rt_ArrayBSearch(&_shaders, &hash, (RtCmpFunc)_shaderModuleCompare);
						
						if (modInfo) {
							s.stages[j].module = modInfo->module;
						} else {
							struct ShaderModuleInfo info =
							{
								.hash = hash,
								.module = Re_deviceProcs.ShaderModule(Re_device, tmp)
							};
							
							if (!info.module) {
								return;
							}
							
							s.stages[j].module = info.module;
							
							Rt_ArrayAdd(&_modules, &info);
							Rt_ArraySort(&_modules, (RtSortFunc)_sortShaderModules);
						}
					} else if (JSON_STRING("stage", mKey, meta.json)) {
						if (JSON_STRING(SSTR_VERTEX, mVal, meta.json))
							s.stages[j].stage = SS_VERTEX;
						else if (JSON_STRING(SSTR_TESS_CTRL, mVal, meta.json))
							s.stages[j].stage = SS_TESS_CTRL;
						else if (JSON_STRING(SSTR_TESS_EVAL, mVal, meta.json))
							s.stages[j].stage = SS_TESS_EVAL;
						else if (JSON_STRING(SSTR_GEOMETRY, mVal, meta.json))
							s.stages[j].stage = SS_GEOMETRY;
						else if (JSON_STRING(SSTR_FRAGMENT, mVal, meta.json))
							s.stages[j].stage = SS_FRAGMENT;
						else if (JSON_STRING(SSTR_COMPUTE, mVal, meta.json))
							s.stages[j].stage = SS_COMPUTE;
						else if (JSON_STRING(SSTR_RAY_GEN, mVal, meta.json))
							s.stages[j].stage = SS_RAYGEN;
						else if (JSON_STRING(SSTR_ANY_HIT, mVal, meta.json))
							s.stages[j].stage = SS_ANY_HIT;
						else if (JSON_STRING(SSTR_CLOSEST_HIT, mVal, meta.json))
							s.stages[j].stage = SS_CLOSEST_HIT;
						else if (JSON_STRING(SSTR_MISS, mVal, meta.json))
							s.stages[j].stage = SS_MISS;
						else if (JSON_STRING(SSTR_INTERSECTION, mVal, meta.json))
							s.stages[j].stage = SS_INTERSECTION;
						else if (JSON_STRING(SSTR_CALLABLE, mVal, meta.json))
							s.stages[j].stage = SS_CALLABLE;
						else if (JSON_STRING(SSTR_TASK, mVal, meta.json))
							s.stages[j].stage = SS_TASK;
						else if (JSON_STRING(SSTR_MESH, mVal, meta.json))
							s.stages[j].stage = SS_MESH;
					}
				} while (pos + 1 < meta.tokenCount && meta.tokens[pos + 1].type != JSMN_OBJECT);
			}
			
			i = pos;
		}
	}
	
	s.hash = Rt_HashString(s.name);
	Rt_ArrayAdd(&_shaders, &s);
}

int32_t
_sortShaders(const struct Shader *a, const struct Shader *b)
{
	if (a->hash == b->hash)
		return 0;
	else if (a->hash > b->hash)
		return -1;
	else
		return 1;
}

int32_t
_shaderCompare(const struct Shader *item, const uint64_t *hash)
{
	if (item->hash == *hash)
		return 0;
	else if (item->hash > *hash)
		return -1;
	else
		return 1;
}
