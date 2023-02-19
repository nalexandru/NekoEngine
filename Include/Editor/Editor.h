#ifndef _NE_EDITOR_EDITOR_H_
#define _NE_EDITOR_EDITOR_H_

#include <Editor/Types.h>
#include <System/PlatformDetect.h>
#include <Engine/Types.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ED_MAX_PATH		4096
#define ED_DIR_SEPARATOR	'/'

enum NeFieldType
{
	FT_UNKNOWN,
	FT_VEC2,
	FT_VEC3,
	FT_VEC4,
	FT_QUAT,
	FT_STRING,
	FT_DOUBLE,
	FT_INT32,
	FT_INT64,
	FT_FLOAT,
	FT_BOOL
};

struct NeDataField
{
	enum NeFieldType type;
	size_t offset;
	char name[32];
};

struct NeComponentFields
{
	NeCompTypeId type;
	uint32_t fieldCount;
	struct NeDataField *fields;
};

extern char Ed_dataDir[];
extern struct NeArray Ed_componentFields;

void Ed_RenderFrame(void);

// This is not part of the exposed IO API
const char *E_RealPath(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* _NE_EDITOR_EDITOR_H_ */

/* NekoEngine
 *
 * Editor.h
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
