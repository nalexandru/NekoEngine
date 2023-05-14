#include <stdlib.h>

#include <jsmn.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Runtime/Json.h>

static inline bool ParseMetadata(struct NeMetadata *meta);

bool
Asset_LoadMetadata(struct NeMetadata *meta, const char *file)
{
	bool rc;
	NeFile f = E_OpenFile(file, IO_READ);
	if (!f)
		return false;
	
	rc = Asset_LoadMetadataFromFile(meta, f);

	E_CloseFile(f);
	
	return rc;
}

bool
Asset_LoadMetadataFromFile(struct NeMetadata *meta, NeFile f)
{
	meta->json = E_ReadFileText(f, &meta->jsonSize, true);
	return ParseMetadata(meta);
}

bool
Asset_LoadMetadataFromStream(struct NeMetadata *meta, struct NeStream *stm)
{
	meta->jsonSize = E_StreamLength(stm);
	meta->json = Sys_Alloc(sizeof(*meta->json), (size_t)meta->jsonSize + 1, MH_Transient);

	E_ReadStream(stm, meta->json, meta->jsonSize);

	return ParseMetadata(meta);
}

void
Asset_LoadMetadataFloatVector(struct NeMetadata *meta, struct jsmntok *tok, float *out, uint32_t count)
{
	uint32_t i;
	char *ptr = meta->json + tok->start;
	for (i = 0; i < count; ++i)
		out[i] = strtof(ptr + (uintptr_t)2 * (i > 0), &ptr); 
}

static inline bool
ParseMetadata(struct NeMetadata *meta)
{
	jsmn_parser p;
	jsmn_init(&p);

	if (!meta->json)
		return false;

	meta->tokenCount = jsmn_parse(&p, meta->json, (uint32_t)meta->jsonSize, NULL, 0);
	if (meta->tokenCount <= 0)
		return false;

	meta->tokens = Sys_Alloc(sizeof(*meta->tokens), meta->tokenCount, MH_Transient);
	if (!meta->tokens)
		return false;

	jsmn_init(&p);
	if (jsmn_parse(&p, meta->json, (uint32_t)meta->jsonSize, meta->tokens, meta->tokenCount) < 0)
		return false;

	if (meta->tokens[0].type != JSMN_OBJECT)
		return false;

	if (!JSON_STRING("id", meta->tokens[1], meta->json))
		return false;

	if (!JSON_STRING(meta->id, meta->tokens[2], meta->json))
		return false;

	if (!JSON_STRING("version", meta->tokens[3], meta->json))
		return false;

	if ((uint32_t)atoi(meta->json + meta->tokens[4].start) != meta->version)
		return false;

	meta->tokenCount -= 5;
	meta->tokens += 5;

	return true;
}

/* NekoEngine
 *
 * Metadata.c
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
