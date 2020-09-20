#include <assert.h>

#include <Engine/IO.h>
#include <Runtime/Json.h>
#include <Runtime/Runtime.h>
#include <Render/Material.h>
#include <System/Memory.h>
#include <System/Log.h>
#include <Engine/Resource.h>
#include <Render/Texture.h>
#include <Render/Shader.h>

#define MATMOD	L"Material"

static void _LoadMaterial(const char *path);

static Array _materials;

static int32_t _SortMaterials(const struct Material *a, const struct Material *b);
static int32_t _CompMaterials(const struct Material *m, const uint64_t *hash);

bool
Re_LoadMaterials(void)
{
	Rt_InitArray(&_materials, 10, sizeof(struct Material));
	E_ProcessFiles("/", "mat", true, _LoadMaterial);

	Rt_ArraySort(&_materials, (RtSortFunc)_SortMaterials);

	return true;
}

void
Re_UnloadMaterials(void)
{
	size_t i = 0, j = 0;
	struct Material *mat;
	for (i = 0; i < _materials.count; ++i) {
		mat = Rt_ArrayGet(&_materials, i);

		free((void *)mat->name);
		free((void *)mat->shader);

		for (j = 0; j < RE_MAX_TEXTURES; ++j) {
			if (!mat->textures[j])
				break;

			if (strncmp(mat->textures[j], "__BlankTexture", strlen(mat->textures[j])))
				free((void *)mat->textures[j]);
		}
	}

	Rt_TermArray(&_materials);
}

bool
Re_CreateMaterial(const wchar_t *name, const wchar_t *shader, const struct MaterialProperties *props, const char *textures[])
{
	int i = 0;
	struct Material *mat = Rt_ArrayAllocate(&_materials);

	mat->name = calloc(wcslen(name) + 1, sizeof(*name));
	memmove((wchar_t *)mat->name, name, sizeof(*name) * wcslen(name));
	mat->hash = Rt_HashStringW(mat->name);
	mat->shaderHash = Rt_HashStringW(shader);

	memmove(&mat->props, props, sizeof(mat->props));

	for (i = 0; i < RE_MAX_TEXTURES; ++i) {
		if (!textures[i])
			continue;

		mat->textures[i] = calloc(strlen(textures[i]) + 1, sizeof(char));
		memmove((char *)mat->textures[i], textures[i], strlen(textures[i]));
	}

	Rt_ArraySort(&_materials, (RtSortFunc)_SortMaterials);

	return true;
}

bool
Re_InstantiateMaterial(const wchar_t *name, struct MaterialInstance *inst)
{
	int i = 0;
	uint64_t hash = 0;
	struct Material *mat = NULL;

	hash = Rt_HashStringW(name);
	mat = Rt_ArrayBSearch(&_materials, &hash, (RtCmpFunc)_CompMaterials);
	if (!mat)
		return false;

	inst->props = mat->props;

	inst->shader = Re_GetShader(mat->shaderHash);

	for (i = 0; i < RE_MAX_TEXTURES; ++i)
		if (mat->textures[i])
			inst->textures[i] = E_LoadResource(mat->textures[i], RES_TEXTURE);
		else
			inst->textures[i] = E_INVALID_HANDLE;

	return true;
}

void
Re_DestroyMaterialInstance(struct MaterialInstance *inst)
{
	int i = 0;
	for (i = 0; i < RE_MAX_TEXTURES; ++i)
		E_UnloadResource(inst->textures[i]);
}

void
_LoadMaterial(const char *path)
{
	jsmn_parser p;
	jsmntok_t *tokens, tok;
	int tokCount = 0, i = 0;
	char *data = NULL, *ptr = NULL;
	int64_t size;
	File f;
	struct Material *mat = Rt_ArrayAllocate(&_materials);

	jsmn_init(&p);

	f = E_OpenFile(path, IO_READ);
	data = E_ReadFileText(f, &size, true);
	E_CloseFile(f);

	tokCount = jsmn_parse(&p, data, (size_t)size, NULL, 0);
	tokens = Sys_Alloc(sizeof(jsmntok_t), tokCount, MH_Transient);
	
	jsmn_init(&p);
	jsmn_parse(&p, data, (size_t)size, tokens, tokCount);

	if (tokens[0].type != JSMN_OBJECT) {
		Sys_LogEntry(MATMOD, LOG_WARNING, L"Invalid material file %S", path);
		return;
	}

	for (i = 0; i < tokCount; ++i) {
		tok = tokens[i];

		if (JSON_STRING("name", tok, data)) {
			tok = tokens[++i];
			mat->name = calloc(((size_t)tok.end - (size_t)tok.start) + 1, sizeof(wchar_t));
			mbstowcs((wchar_t *)mat->name, data + tok.start, (size_t)tok.end - (size_t)tok.start);
			mat->hash = Rt_HashStringW(mat->name);
		} else if (JSON_STRING("shader", tok, data)) {
			tok = tokens[++i];
			mat->shader = calloc(((size_t)tok.end - (size_t)tok.start) + 1, sizeof(wchar_t));
			mbstowcs((wchar_t *)mat->shader, data + tok.start, (size_t)tok.end - (size_t)tok.start);
			mat->shaderHash = Rt_HashStringW(mat->shader);
		} else if (JSON_STRING("color", tok, data)) {
			tok = tokens[++i];
			mat->props.color.r = strtof(data + tok.start, &ptr);
			mat->props.color.g = strtof(ptr + 2, &ptr);
			mat->props.color.b = strtof(ptr + 2, &ptr);
			mat->props.color.a = strtof(ptr + 2, &ptr);
		} else if (JSON_STRING("emissive", tok, data)) {
			tok = tokens[++i];
			mat->props.emissive.r = strtof(data + tok.start, &ptr);
			mat->props.emissive.g = strtof(ptr + 2, &ptr);
			mat->props.emissive.b = strtof(ptr + 2, &ptr);
		} else if (JSON_STRING("roughness", tok, data)) {
			tok = tokens[++i];
			mat->props.roughness = strtof(data + tok.start, NULL);
		} else if (JSON_STRING("metallic", tok, data)) {
			tok = tokens[++i];
			mat->props.metallic = strtof(data + tok.start, NULL);
		} else if (JSON_STRING("transparent", tok, data)) {
			tok = tokens[++i];
			mat->props.alphaMode = !strncmp(data + tok.start, "true", 4) ? ALPHA_MODE_BLEND : ALPHA_MODE_OPAQUE;
		} else if (JSON_STRING("diffuseMap", tok, data)) {
			tok = tokens[++i];
			mat->textures[MAP_DIFFUSE] = calloc(((size_t)tok.end - (size_t)tok.start) + 1, sizeof(char));
			memmove((char *)mat->textures[MAP_DIFFUSE], data + tok.start, (size_t)tok.end - (size_t)tok.start);
		} else if (JSON_STRING("normalMap", tok, data)) {
			tok = tokens[++i];
			mat->textures[MAP_NORMAL] = calloc(((size_t)tok.end - (size_t)tok.start) + 1, sizeof(char));
			memmove((char *)mat->textures[MAP_NORMAL], data + tok.start, (size_t)tok.end - (size_t)tok.start);
		} else if (JSON_STRING("metallicRoughnessMap", tok, data)) {
			tok = tokens[++i];
			mat->textures[MAP_METALLIC_ROUGHNESS] = calloc(((size_t)tok.end - (size_t)tok.start) + 1, sizeof(char));
			memmove((char *)mat->textures[MAP_METALLIC_ROUGHNESS], data + tok.start, (size_t)tok.end - (size_t)tok.start);
		} else if (JSON_STRING("emissiveMap", tok, data)) {
			tok = tokens[++i];
			mat->textures[MAP_EMISSIVE] = calloc(((size_t)tok.end - (size_t)tok.start) + 1, sizeof(char));
			memmove((char *)mat->textures[MAP_EMISSIVE], data + tok.start, (size_t)tok.end - (size_t)tok.start);
		}
	}

	return;
}

static int32_t
_SortMaterials(const struct Material *a, const struct Material *b)
{
	return a->hash > b->hash ? 1 : (a->hash < b->hash ? -1 : 0);
}

static int32_t
_CompMaterials(const struct Material *m, const uint64_t *hash)
{
	return m->hash > *hash ? 1 : (m->hash < *hash ? -1 : 0);
}
