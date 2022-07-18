#ifndef _NE_ENGINE_CONFIG_H_
#define _NE_ENGINE_CONFIG_H_

#include <Engine/Types.h>

#define CONF_PATH_SIZE	256
#define CVAR_MAX_NAME	64

enum NeCVarType
{
	CV_String,
	CV_Int32,
	CV_UInt32,
	CV_UInt64,
	CV_Float,
	CV_Bool
};

struct NeCVar
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
	enum NeCVarType type;
	char name[CVAR_MAX_NAME];
	struct NeCVar *next;
};

void E_InitConfig(const char *file);
void E_TermConfig(void);

const struct NeCVar *E_RootCVar(void);
struct NeCVar *E_GetCVar(const char *name);

#define CVAR_STRING(x) E_GetCVarStr(x, NULL)->str
#define CVAR_INT32(x) E_GetCVarI32(x, 0)->i32
#define CVAR_UINT32(x) E_GetCVarU32(x, 0)->u32
#define CVAR_UINT64(x) E_GetCVarU64(x, 0)->u64
#define CVAR_FLOAT(x) E_GetCVarFlt(x, 0.f)->flt
#define CVAR_BOOL(x) E_GetCVarBln(x, false)->bln

struct NeCVar *E_GetCVarStr(const char *name, const char *def);
struct NeCVar *E_GetCVarI32(const char *name, int32_t def);
struct NeCVar *E_GetCVarU32(const char *name, uint32_t def);
struct NeCVar *E_GetCVarU64(const char *name, uint64_t def);
struct NeCVar *E_GetCVarFlt(const char *name, float def);
struct NeCVar *E_GetCVarBln(const char *name, bool def);

void E_SetCVarStr(const char *name, const char *str);
void E_SetCVarI32(const char *name, int32_t i32);
void E_SetCVarU32(const char *name, uint32_t u32);
void E_SetCVarU64(const char *name, uint64_t u64);
void E_SetCVarFlt(const char *name, float flt);
void E_SetCVarBln(const char *name, bool bln);

#endif /* _NE_ENGINE_CONFIG_H_ */
