#ifndef _E_CONFIG_H_
#define _E_CONFIG_H_

#include <Engine/Types.h>

#define CONF_PATH_SIZE	256
#define CVAR_MAX_NAME	64

enum CVarType
{
	CV_String,
	CV_Int32,
	CV_UInt32,
	CV_UInt64,
	CV_Float,
	CV_Bool
};

struct CVar
{
	union {
		const char *str;
		int32_t i32;
		uint32_t u32;
		uint64_t u64;
		float flt;
		bool bln;
	};
	uint64_t hash;
	enum CVarType type;
	wchar_t name[CVAR_MAX_NAME];
	struct CVar *next;
};

void E_InitConfig(const char *file);
void E_TermConfig(void);

#define CVAR_STRING(x) E_GetCVarStr(x, NULL)->str
#define CVAR_INT32(x) E_GetCVarI32(x, 0)->i32
#define CVAR_UINT32(x) E_GetCVarU32(x, 0)->u32
#define CVAR_UINT64(x) E_GetCVarU64(x, 0)->u64
#define CVAR_FLOAT(x) E_GetCVarFlt(x, 0.f)->flt
#define CVAR_BOOL(x) E_GetCVarBln(x, false)->bln

struct CVar *E_GetCVarStr(const wchar_t *name, const char *def);
struct CVar *E_GetCVarI32(const wchar_t *name, int32_t def);
struct CVar *E_GetCVarU32(const wchar_t *name, uint32_t def);
struct CVar *E_GetCVarU64(const wchar_t *name, uint64_t def);
struct CVar *E_GetCVarFlt(const wchar_t *name, float def);
struct CVar *E_GetCVarBln(const wchar_t *name, bool def);

void E_SetCVarStr(const wchar_t *name, const char *str);
void E_SetCVarI32(const wchar_t *name, int32_t i32);
void E_SetCVarU32(const wchar_t *name, uint32_t u32);
void E_SetCVarU64(const wchar_t *name, uint64_t u64);
void E_SetCVarFlt(const wchar_t *name, float flt);
void E_SetCVarBln(const wchar_t *name, bool bln);

#endif /* _E_CONFIG_H_ */
