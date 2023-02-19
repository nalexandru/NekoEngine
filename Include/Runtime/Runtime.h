#ifndef _NE_RUNTIME_RUNTIME_H_
#define _NE_RUNTIME_RUNTIME_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Runtime/RtDefs.h>
#include <Runtime/Array.h>
#include <Runtime/Queue.h>

#include <System/System.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline char *
Rt_SkipWhitespace(char *str)
{
	while (*str) {
		if (isspace(*str))
			++str;
		else
			break;
	}
	return str;
}

static inline char *
Rt_StrDup(const char *str, enum NeMemoryHeap mh)
{
	const size_t len = strlen(str);
	char *ret = (char *)Sys_Alloc(sizeof(*ret), len + 1, mh);
	strncpy(ret, str, len);
	return ret;
}

static inline char *
Rt_StrNDup(const char *str, size_t maxCount, enum NeMemoryHeap mh)
{
	const size_t len = strnlen(str, maxCount);
	char *ret = (char *)Sys_Alloc(sizeof(*ret), len + 1, mh);
	strncpy(ret, str, len);
	return ret;
}

static inline char *
Rt_TransientStrDup(const char *str)
{
	return Rt_StrDup(str, MH_Transient);
}

static inline bool
Rt_IsNumeric(const char *str)
{
	while (*str)
		if (!isdigit(*str++))
			return false;
	return true;
}

#ifdef __cplusplus
}
#endif

#endif /* _NE_RUNTIME_RUNTIME_H_ */

/* NekoEngine
 *
 * Runtime.h
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
