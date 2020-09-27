#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <System/System.h>
#include <Runtime/Runtime.h>
#include <Engine/Config.h>

#define BUFF_SZ	512

static struct CVar *_cvars = NULL;

static inline struct CVar *_findCVar(const wchar_t *name, enum CVarType type);
static inline struct CVar *_newCVar(const wchar_t *name, enum CVarType type);

void
E_InitConfig(const char *file)
{
	FILE *fp = NULL;
	char buff[BUFF_SZ], val[BUFF_SZ], section[BUFF_SZ];
	wchar_t name[CVAR_MAX_NAME];

	if (!file)
		return;

	fp = fopen(file, "r");
	if (!fp)
		return;

	memset(val, 0x0, BUFF_SZ);
	memset(buff, 0x0, BUFF_SZ);
	memset(section, 0x0, BUFF_SZ);

	while (fgets(buff, BUFF_SZ, fp)) {
		char *key = NULL, *ptr = NULL;

		if (buff[0] == '\n' || buff[0] == '\r')
			continue;

		ptr = buff + strlen(buff) - 1;
		while (isspace(*ptr))
			*ptr-- = 0x0;
	
		ptr = buff;
		while (*ptr && isspace(*ptr))
			++ptr;

		if (sscanf(ptr, "[%s]", section)) {
			section[BUFF_SZ - 1] = 0x0;
			ptr = strchr(section, ']');
			*ptr = 0x0;
		} else {
			key = ptr;
			ptr = strchr(ptr, '=');
			*ptr++ = 0x0;

			swprintf(name, CVAR_MAX_NAME, L"%hs_%hs", section, &key[1]);
			switch (key[0]) {
			case 's': E_GetCVarStr(name, ptr); break;
			case 'i': E_GetCVarI32(name, atoi(ptr)); break;
			case 'u': E_GetCVarU32(name, strtoul(ptr, NULL, 10)); break;
			case 'l': E_GetCVarU64(name, strtoull(ptr, NULL, 10)); break;
			case 'f': E_GetCVarFlt(name, strtof(ptr, NULL)); break;
			case 'b': E_GetCVarBln(name, !strncmp(ptr, "true", strlen(ptr))); break;
			}
		}

		memset(val, 0x0, BUFF_SZ);
		memset(buff, 0x0, BUFF_SZ);
		memset(name, 0x0, CVAR_MAX_NAME * sizeof(wchar_t));
	}

	fclose(fp);
}

void
E_TermConfig(void)
{
	struct CVar *cv = _cvars, *prev;
	while (cv) {
		if (cv->type == CV_String)
			free((void *)cv->str);

		prev = cv;
		cv = cv->next;

		free(prev);
	}
}

#define GET_CVAR_IMPL(suffix, member, type, cvtype)			\
struct CVar *												\
E_GetCVar ## suffix(const wchar_t *name, type def)			\
{															\
	struct CVar *cv = _findCVar(name, cvtype);				\
	if (!cv) {												\
		cv = _newCVar(name, cvtype);						\
		cv->member = def;									\
	}														\
	return cv;												\
}

#define GETS_CVAR_IMPL(suffix, member, type, cvtype, set)	\
struct CVar *												\
E_GetCVar ## suffix(const wchar_t *name, type def)			\
{															\
	struct CVar *cv = _findCVar(name, cvtype);				\
	if (!cv) {												\
		cv = _newCVar(name, cvtype);						\
		cv->member = set(def);								\
	}														\
	return cv;												\
}

#define SET_CVAR_IMPL(suffix, member, type, cvtype)			\
void														\
E_SetCVar ## suffix(const wchar_t *name, type val)			\
{															\
	struct CVar *cv = E_GetCVar ## suffix(name, val);		\
	cv->member = val;										\
}

#define SETF_CVAR_IMPL(suffix, member, type, cvtype, set)	\
void														\
E_SetCVar ## suffix(const wchar_t *name, type val)			\
{															\
	struct CVar *cv = E_GetCVar ## suffix(name, val);		\
	free((void *)cv->member);								\
	cv->member = set(val);									\
}

GETS_CVAR_IMPL(Str, str, const char *, CV_String, strdup)
GET_CVAR_IMPL(I32, i32, int32_t, CV_Int32)
GET_CVAR_IMPL(U32, u32, uint32_t, CV_UInt32)
GET_CVAR_IMPL(U64, u64, uint64_t, CV_UInt64)
GET_CVAR_IMPL(Flt, flt, float, CV_Float)
GET_CVAR_IMPL(Bln, bln, bool, CV_Bool)

SETF_CVAR_IMPL(Str, str, const char *, CV_String, strdup)
SET_CVAR_IMPL(I32, i32, int32_t, CV_Int32)
SET_CVAR_IMPL(U32, u32, uint32_t, CV_UInt32)
SET_CVAR_IMPL(U64, u64, uint64_t, CV_UInt64)
SET_CVAR_IMPL(Flt, flt, float, CV_Float)
SET_CVAR_IMPL(Bln, bln, bool, CV_Bool)

struct CVar *
_findCVar(const wchar_t *name, enum CVarType type)
{
	uint64_t hash = 0;
	struct CVar *cv = _cvars;

	hash = Rt_HashStringW(name);

	while (cv) {
		if (cv->hash == hash && cv->type == type)
			return cv;
		cv = cv->next;
	}

	return NULL;
}

struct CVar *
_newCVar(const wchar_t *name, enum CVarType type)
{
	uint64_t hash = 0;
	struct CVar *cv = _cvars;

	hash = Rt_HashStringW(name);

	if (!cv) {
		_cvars = calloc(1, sizeof(*cv));
		cv = _cvars;
	} else {
		while (cv) {
			if (cv->next) {
				cv = cv->next;
				continue;
			}

			cv->next = calloc(1, sizeof(*cv));
			cv = cv->next;
			break;
		}
	}

	assert(cv);

	cv->type = type;
	cv->hash = hash;
	wcsncpy(cv->name, name, CVAR_MAX_NAME);

	return cv;
}

