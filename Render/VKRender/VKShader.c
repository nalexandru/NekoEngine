#include <assert.h>

#include <Engine/IO.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Render/Shader.h>
#include <Runtime/Json.h>

#include "VKRender.h"

#define VKSMOD			L"VulkanShader"
#define SH_PATH			"/Shaders/HLSL6"
#define SH_PATH_FMT		"/Shaders/HLSL6/%s"

Array _shaders;
static uint64_t _pinkShader;
/*static void _LoadShader(const char *path);
static inline bool _LoadBytecode(const char *path, D3D12_SHADER_BYTECODE *out);
static inline void _UnloadShader(struct Shader *s);*/
static int32_t _SortShaders(const struct Shader *a, const struct Shader *b);
static int32_t _CompShaders(const struct Shader *m, const uint64_t *hash);

void *
VK_GetShader(uint64_t hash)
{
	void *ret = Rt_ArrayBSearch(&_shaders, &hash, (RtCmpFunc)_CompShaders);
	if (!ret)
		ret = Rt_ArrayBSearch(&_shaders, &_pinkShader, (RtCmpFunc)_CompShaders);

	return ret;
}

bool
D3D12_LoadShaders(void)
{
	/*Rt_InitArray(&_shaders, 10, sizeof(struct Shader));
	E_ProcessFiles(SH_PATH, "shader", true, _LoadShader);

	Rt_ArraySort(&_shaders, (RtSortFunc)_SortShaders);

	_pinkShader = Rt_HashStringW(L"PinkShader");*/

	return true;
}

void
D3D12_UnloadShaders(void)
{
/*	for (size_t i = 0; i < _shaders.count; ++i)
		_UnloadShader((struct Shader *)Rt_ArrayGet(&_shaders, i));

	Rt_TermArray(&_shaders);*/
}

/*void
_LoadShader(const char *path)
{
	uint64_t size;
	char *data, *buff;
	uint32_t tokCount;
	jsmntok_t *tokens, tok;
	jsmn_parser p;
	bool ret = true;

	struct Shader *s = (struct Shader *)Rt_ArrayAllocate(&_shaders);

	data = (char *)E_MapFile(path, IO_READ, &size);

	jsmn_init(&p);
	tokCount = jsmn_parse(&p, data, (size_t)size, NULL, 0);
	tokens = (jsmntok_t *)Sys_Alloc(sizeof(jsmntok_t), tokCount, MH_Transient);
	
	jsmn_init(&p);
	jsmn_parse(&p, data, (size_t)size, tokens, tokCount);

	if (tokens[0].type != JSMN_OBJECT) {
		Sys_LogEntry(D3D12SMOD, LOG_CRITICAL, L"Invalid shader definition file %S", path);
		return;
	}

	buff = (char *)Sys_Alloc(sizeof(char), 256, MH_Transient);

	for (uint32_t i = 1; i < tokCount; ++i) {
		tok = tokens[i];

		if (JSON_STRING("name", tok, data)) {
			tok = tokens[++i];
			mbstowcs(s->name, data + tok.start, (size_t)tok.end - (size_t)tok.start);
			s->hash = Rt_HashStringW(s->name);
		} else if (JSON_STRING("type", tok, data)) {
			tok = tokens[++i];

			if (JSON_STRING("graphics", tok, data))
				s->type = ST_Graphics;
			else if (JSON_STRING("library", tok, data))
				s->type = ST_Library;
			else if (JSON_STRING("mesh", tok, data))
				s->type = ST_Mesh;
			if (JSON_STRING("compute", tok, data))
				s->type = ST_Compute;
		} else if (JSON_STRING("vertex", tok, data) && s->type == ST_Graphics) {
			tok = tokens[++i];
			snprintf(buff, 16 + (size_t)tok.end - (size_t)tok.start, SH_PATH_FMT, data + tok.start);
			_LoadBytecode(buff, &s->VS);
		} else if (JSON_STRING("pixel", tok, data) && (s->type == ST_Graphics || s->type == ST_Mesh)) {
			tok = tokens[++i];
			snprintf(buff, 16 + (size_t)tok.end - (size_t)tok.start, SH_PATH_FMT, data + tok.start);
			ret = _LoadBytecode(buff, &s->PS);
		} else if (JSON_STRING("mesh", tok, data) && s->type == ST_Mesh) {
			tok = tokens[++i];
			snprintf(buff, 16 + (size_t)tok.end - (size_t)tok.start, SH_PATH_FMT, data + tok.start);
			ret = _LoadBytecode(buff, &s->MS);
		} else if (JSON_STRING("amplification", tok, data) && s->type == ST_Mesh) {
			tok = tokens[++i];
			snprintf(buff, 16 + (size_t)tok.end - (size_t)tok.start, SH_PATH_FMT, data + tok.start);
			ret = _LoadBytecode(buff, &s->AS);
		} else if (JSON_STRING("geometry", tok, data) && s->type == ST_Graphics) {
			tok = tokens[++i];
			snprintf(buff, 16 + (size_t)tok.end - (size_t)tok.start, SH_PATH_FMT, data + tok.start);
			ret = _LoadBytecode(buff, &s->GS);
		} else if (JSON_STRING("hull", tok, data) && s->type == ST_Graphics) {
			tok = tokens[++i];
			snprintf(buff, 16 + (size_t)tok.end - (size_t)tok.start, SH_PATH_FMT, data + tok.start);
			ret = _LoadBytecode(buff, &s->HS);
		} else if (JSON_STRING("domain", tok, data) && s->type == ST_Graphics) {
			tok = tokens[++i];
			snprintf(buff, 16 + (size_t)tok.end - (size_t)tok.start, SH_PATH_FMT, data + tok.start);
			ret = _LoadBytecode(buff, &s->DS);
		} else if (JSON_STRING("compute", tok, data) && s->type == ST_Compute) {
			tok = tokens[++i];
			ret = snprintf(buff, 16 + (size_t)tok.end - (size_t)tok.start, SH_PATH_FMT, data + tok.start);
			ret = _LoadBytecode(buff, &s->CS);
		} else if (JSON_STRING("library", tok, data) && s->type == ST_Library) {
			tok = tokens[++i];
			snprintf(buff, 16 + (size_t)tok.end - (size_t)tok.start, SH_PATH_FMT, data + tok.start);
			ret = _LoadBytecode(buff, &s->lib);
		}

		if (!ret) {
			E_UnmapFile(data, size);
			goto error;
		}
	}

	E_UnmapFile(data, size);

	if (s->type != ST_Invalid)
		return;

error:
	Sys_LogEntry(D3D12SMOD, LOG_CRITICAL, L"Failed to load shader from file %S", path);
	_UnloadShader(s);
	memset(s, 0x0, sizeof(*s));
	--_shaders.count;
}

bool
_LoadBytecode(const char *path, D3D12_SHADER_BYTECODE *out)
{
	File f;
	int64_t size;

	f = E_OpenFile(path, IO_READ);
	if (!f)
		return false;
	
	out->pShaderBytecode = E_ReadFileBlob(f, &size, false);
	E_CloseFile(f);

	if (!out->pShaderBytecode)
		return false;
	
	out->BytecodeLength = (SIZE_T)size;

	return true;
}

void _UnloadShader(struct Shader *s)
{
	if (s->type == ST_Graphics) {
		Sys_Free((void *)s->VS.pShaderBytecode);
		Sys_Free((void *)s->PS.pShaderBytecode);
		Sys_Free((void *)s->DS.pShaderBytecode);
		Sys_Free((void *)s->HS.pShaderBytecode);
		Sys_Free((void *)s->GS.pShaderBytecode);
	} else if (s->type == ST_Mesh) {
		Sys_Free((void *)s->AS.pShaderBytecode);
		Sys_Free((void *)s->MS.pShaderBytecode);
		Sys_Free((void *)s->PS.pShaderBytecode);
	} else if (s->type == ST_Compute) {
		Sys_Free((void *)s->CS.pShaderBytecode);
	} else if (s->type == ST_Library) {
		Sys_Free((void *)s->lib.pShaderBytecode);
	}
}*/

int32_t
_SortShaders(const struct Shader *a, const struct Shader *b)
{
	return 0; //a->hash > b->hash ? 1 : (a->hash < b->hash ? -1 : 0);
}

int32_t
_CompShaders(const struct Shader *m, const uint64_t *hash)
{
	return 0;// m->hash > *hash ? 1 : (m->hash < *hash ? -1 : 0);
}
