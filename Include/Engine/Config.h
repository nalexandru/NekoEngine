#ifndef NE_ENGINE_CONFIG_H
#define NE_ENGINE_CONFIG_H

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifndef __cplusplus

#define E_SetCVar(x, y) _Generic((x),	\
	const char *: E_SetCVarStr,	\
	int32_t: E_SetCVarI32,	\
	uint32_t: E_SetCVarU32,	\
	uint64_t: E_SetCVarU64,	\
	float: E_SetCVarFlt	\
	bool: E_SetCVarBln	\
)(x, y)

#endif

#ifdef __cplusplus
}
#endif

#endif /* NE_ENGINE_CONFIG_H */

/* NekoEngine
 *
 * Config.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
