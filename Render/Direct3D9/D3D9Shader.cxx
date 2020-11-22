#include <stdio.h>

#include <Engine/IO.h>
#include <System/Log.h>
#include <Runtime/Json.h>
#include <System/Memory.h>
#include <System/System.h>
#include <Render/Model.h>
#include <Render/Device.h>
#include <Render/Shader.h>

#include "D3D9Render.h"

#define D3D9SMOD		L"D3D9Shader"
#define SH_PATH			"/Shaders/SM3"
#define SH_PATH_FMT		"/Shaders/SM3/%s"
#define SH_PATH_FMT_SZ	sizeof(SH_PATH_FMT) - 2

Array _shaders;

static uint64_t _defaultHash;
static void _LoadProgram(const char *path);
static inline DWORD *_LoadBytecode(const char *path);
static inline void _UnloadShader(struct Shader *s);
static int32_t _SortShaders(const struct Shader *a, const struct Shader *b);
static int32_t _CompShaders(const struct Shader *m, const uint64_t *hash);

void *
D3D9_GetShader(uint64_t hash)
{
	void *ret = Rt_ArrayBSearch(&_shaders, &hash, (RtCmpFunc)_CompShaders);
	if (!ret)
		ret = Rt_ArrayBSearch(&_shaders, &_defaultHash, (RtCmpFunc)_CompShaders);

	return ret;
}

bool
D3D9_LoadShaders(void)
{
	Rt_InitArray(&_shaders, 10, sizeof(struct Shader));
	E_ProcessFiles(SH_PATH, "shader", true, _LoadProgram);

	Rt_ArraySort(&_shaders, (RtSortFunc)_SortShaders);

	_defaultHash = Rt_HashStringW(L"Default");

	return true;
}

void
D3D9_UnloadShaders(void)
{
	for (size_t i = 0; i < _shaders.count; ++i)
		_UnloadShader((struct Shader *)Rt_ArrayGet(&_shaders, i));
}

static void
_LoadProgram(const char *path)
{
	int64_t size;
	jsmn_parser p;

	struct Shader *s = (struct Shader *)Rt_ArrayAllocate(&_shaders);

	File f = E_OpenFile(path, IO_READ);
	char *data = E_ReadFileText(f, &size, true);
	E_CloseFile(f);

	jsmn_init(&p);
	uint32_t tokCount = jsmn_parse(&p, data, (size_t)size, NULL, 0);
	jsmntok_t *tokens = (jsmntok_t *)Sys_Alloc(sizeof(jsmntok_t), tokCount, MH_Transient);
	
	jsmn_init(&p);
	jsmn_parse(&p, data, (size_t)size, tokens, tokCount);

	if (tokens[0].type != JSMN_OBJECT) {
		Sys_LogEntry(D3D9SMOD, LOG_WARNING, L"Invalid shader definition file %S", path);
		return;
	}

	wchar_t *wbuff = (wchar_t *)Sys_Alloc(sizeof(wchar_t), 64, MH_Transient);
	char *buff = (char *)Sys_Alloc(sizeof(char), 256, MH_Transient);

	for (uint32_t i = 1; i < tokCount; ++i) {
		HRESULT hr = S_OK;
		jsmntok_t tok = tokens[i];

		if (JSON_STRING("name", tok, data)) {
			tok = tokens[++i];
			mbstowcs(wbuff, data + tok.start, (size_t)tok.end - (size_t)tok.start);
			s->hash = Rt_HashStringW(wbuff);
		} else if (JSON_STRING("vertex", tok, data)) {
			tok = tokens[++i];
			snprintf(buff, SH_PATH_FMT_SZ + (size_t)tok.end - (size_t)tok.start, SH_PATH_FMT, data + tok.start);
			const DWORD *blob = _LoadBytecode(buff);
			hr = blob ? Re_Device.dev->CreateVertexShader(blob, &s->vs) : S_FALSE;
		} else if (JSON_STRING("pixel", tok, data)) {
			tok = tokens[++i];
			snprintf(buff, SH_PATH_FMT_SZ + (size_t)tok.end - (size_t)tok.start, SH_PATH_FMT, data + tok.start);
			const DWORD *blob = _LoadBytecode(buff);
			hr = blob ? Re_Device.dev->CreatePixelShader(blob, &s->ps) : S_FALSE;
		}

		if (FAILED(hr))
			goto error;
	}

	if (s->hash)
		return;

error:
	Sys_LogEntry(D3D9SMOD, LOG_CRITICAL, L"Failed to load shader from file %S", path);
	_UnloadShader(s);
	memset(s, 0x0, sizeof(*s));
	--_shaders.count;
}

DWORD *
_LoadBytecode(const char *path)
{
	File f = E_OpenFile(path, IO_READ);
	if (!f)
		return NULL;

	int64_t size;
	void *ret = E_ReadFileBlob(f, &size, true);

	E_CloseFile(f);

	return (DWORD *)ret;
}

void
_UnloadShader(struct Shader *s)
{
	if (s->vs)
		s->vs->Release();
	if (s->ps)
		s->ps->Release();
}

static int32_t
_SortShaders(const struct Shader *a, const struct Shader *b)
{
	return a->hash > b->hash ? 1 : (a->hash < b->hash ? -1 : 0);
}

static int32_t
_CompShaders(const struct Shader *m, const uint64_t *hash)
{
	return m->hash > *hash ? 1 : (m->hash < *hash ? -1 : 0);
}
