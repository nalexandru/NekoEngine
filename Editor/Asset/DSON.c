#include <stdio.h>

#include <Editor/Types.h>
#include <Runtime/Json.h>
#include <System/Memory.h>
#include <System/Log.h>

#include <Editor/Asset/Asset.h>
#include <Editor/Asset/Import.h>

#define DSONIMPMOD	"DSONImporter"

// http://docs.daz3d.com/doku.php/public/dson_spec/start

struct DSONData
{
	jsmntok_t *tokens;
	uint32_t tokenCount;
	char *json;
};

ED_ASSET_IMPORTER(DSON);

static inline uint32_t ParseGeometryLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);
static inline uint32_t ParseNodeLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);
static inline uint32_t ParseUVSetLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);
static inline uint32_t ParseMaterialLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);
static inline uint32_t ParseModifierLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);
static inline uint32_t ParseImageLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);

static bool
DSON_MatchAsset(const char *path)
{
	return strstr(path, ".duf") != NULL;
}

static bool
DSON_ImportAsset(const char *path, const struct NeAssetImportOptions *options)
{
	bool rc = false;
	jsmn_parser p;
	jsmn_init(&p);
	struct DSONData data = { 0 };

	FILE *fp = fopen(path, "r");
	if (!fp)
		goto exit;

	fseek(fp, 0, SEEK_END);
	uint32_t jsonSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	data.json = Sys_Alloc(sizeof(*data.json), jsonSize, MH_Asset);

	uint32_t tokenCount = jsmn_parse(&p, data.json, jsonSize, NULL, 0);
	if (tokenCount <= 0)
		goto exit;

	data.tokens = Sys_Alloc(sizeof(*data.tokens), tokenCount, MH_Asset);
	if (!data.tokens)
		goto exit;

	jsmn_init(&p);
	if (jsmn_parse(&p, data.json, jsonSize, data.tokens, tokenCount) < 0)
		goto exit;

	if (data.tokens[0].type != JSMN_OBJECT)
		return false;

	for (uint32_t i = 0; i < tokenCount; ++i) {
		jsmntok_t key = data.tokens[i];
		jsmntok_t val = data.tokens[++i];

		if (JSON_STRING("file_version", key, data.json)) {
			if (!JSON_STRING("0.6.0.0", val, data.json) && !JSON_STRING("0.6.1.0", val, data.json)) {
				Sys_LogEntry(DSONIMPMOD, LOG_CRITICAL, "Unknown DSON version %s", data.json + val.start);
				goto exit;
			}
		} else if (JSON_STRING("asset_info", key, data.json)) {

		} else if (JSON_STRING("geometry_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = ParseGeometryLibrary(i, val.size, &data);
		} else if (JSON_STRING("node_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = ParseNodeLibrary(i, val.size, &data);
		} else if (JSON_STRING("uv_set_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = ParseUVSetLibrary(i, val.size, &data);
		} else if (JSON_STRING("modifier_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = ParseModifierLibrary(i, val.size, &data);
		} else if (JSON_STRING("image_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = ParseImageLibrary(i, val.size, &data);
		} else if (JSON_STRING("material_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = ParseMaterialLibrary(i, val.size, &data);
		} else if (JSON_STRING("scene", key, data.json)) {
			if (val.type != JSMN_OBJECT)
				continue;
		} else {
		}
	}

	rc = true;

exit:
	Sys_Free(data.json);
	Sys_Free(data.tokens);

	return rc;
}

static inline
uint32_t ParseGeometryLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

			if (JSON_STRING("id", key, data->json)) {
			} else if (JSON_STRING("vertices", key, data->json)) {
			} else if (JSON_STRING("polygon_groups", key, data->json)) {
			} else if (JSON_STRING("polygon_material_groups", key, data->json)) {
			} else if (JSON_STRING("polylist", key, data->json)) {
			}
		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}

static inline
uint32_t ParseNodeLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			//const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}

static inline
uint32_t ParseUVSetLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			//const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}

static inline
uint32_t ParseMaterialLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			//const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}

static inline
uint32_t ParseModifierLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			//const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}

static inline
uint32_t ParseImageLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			//const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}

/* NekoEditor
 *
 * DSON.c
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
