/* NekoEngine
 *
 * texture.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Texture
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 */

#include <stdlib.h>

#include <system/log.h>
#include <system/compat.h>

#include <engine/io.h>
#include <engine/json.h>
#include <engine/asset.h>
#include <graphics/texture.h>
#include <graphics/graphics.h>

#define TEXTURE_MODULE	"Texture"

#define TEX_FMT_DDS	0
#define TEX_FMT_TGA	1

void *
load_texture(const char *path)
{
	struct ne_texture *tex = NULL;
	struct ne_texture_create_info ci;
	ne_status ret = NE_FAIL;
	ne_file *file = NULL;
	int64_t size = 0;
	uint8_t *data = NULL;
	char *metadata= NULL;
	const char *image_path = NULL;
	ne_texture_format format = NE_TEXTURE_FORMAT_DDS;
	cJSON *json = NULL;
	const cJSON *tmp = NULL;
	bool free_data = false;
	uint8_t *img_data = NULL;
	uint64_t img_size;
	char meta_path[PATH_MAX];

	if (!strstr(path, ".nemesh"))
		snprintf(meta_path, PATH_MAX, "%s.netex", path);
	else
		snprintf(meta_path, PATH_MAX, "%s", path);

	memset(&ci, 0x0, sizeof(ci));

	file = io_open(meta_path, IO_READ);
	if (!file) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Failed to open file [%s]", meta_path);
		goto error;
	}

	metadata = io_read_text(file, &size);
	if (!metadata) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Failed to read file [%s]", meta_path);
		goto error;
	}

	io_close(file);
	file = NULL;

	json = cJSON_Parse(metadata);
	if (!json) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Failed to parse file [%s]: %s",
			meta_path, cJSON_GetErrorPtr());
		goto error;
	}

	if (!cJSON_IsObject(json)) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No root object", meta_path);
		goto error;
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "format");
	if (!cJSON_IsString(tmp) || !tmp->valuestring) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No file format", meta_path);
		goto error;
	}

	if (!strncmp(tmp->valuestring, "DDS", strlen(tmp->valuestring))) {
		format = NE_TEXTURE_FORMAT_DDS;
	} else if (!strncmp(tmp->valuestring, "TGA", strlen(tmp->valuestring))) {
		format = NE_TEXTURE_FORMAT_TGA;
	} else if (!strncmp(tmp->valuestring, "PNG", strlen(tmp->valuestring))) {
		format = NE_TEXTURE_FORMAT_PNG;
	} else {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Unknown texture file format [%s] for [%s]",
			tmp->valuestring, meta_path);
		return NULL;
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "type");
	if (!cJSON_IsString(tmp) || !tmp->valuestring) {
		if (format != NE_TEXTURE_FORMAT_DDS) {
			log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No texture type", meta_path);
			goto error;
		}
	} else {
		if (!strncmp(tmp->valuestring, "1D", strlen(tmp->valuestring))) {
			ci.type = NE_TEXTURE_1D;
		} else if (!strncmp(tmp->valuestring, "2D", strlen(tmp->valuestring))) {
			ci.type = NE_TEXTURE_2D;
		} else if (!strncmp(tmp->valuestring, "3D", strlen(tmp->valuestring))) {
			ci.type = NE_TEXTURE_3D;
		} else if (!strncmp(tmp->valuestring, "cubemap", strlen(tmp->valuestring))) {
			ci.type = NE_TEXTURE_CUBEMAP;
		} else if (format != NE_TEXTURE_FORMAT_DDS) {
			log_entry(TEXTURE_MODULE, LOG_CRITICAL,
				"Unknown texture type [%s] for [%s]",
				tmp->valuestring, meta_path);
			return NULL;
		}
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "path");
	if (!cJSON_IsString(tmp) || !tmp->valuestring) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No file path", meta_path);
		goto error;
	}
	image_path = tmp->valuestring;

	file = io_open(image_path, IO_READ);
	if (!file) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
				"Failed to open file [%s]", meta_path);
		ret = NE_IO_FAIL;
		goto error;
	}

	data = io_read_blob(file, &size);
	if (!data) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
				"Failed to read file [%s]", meta_path);
		ret = NE_IO_FAIL;
		goto error;
	}

	switch (format) {
	case NE_TEXTURE_FORMAT_DDS:
		ret = asset_load_dds(data, size, &ci.width, &ci.height,
			&ci.depth, &ci.type, &ci.format,
			&ci.levels, &ci.layers, &img_data,
			&img_size, &free_data);
		break;
	case NE_TEXTURE_FORMAT_TGA:
		ret = asset_load_tga(data, size, &ci.width, &ci.height,
			&ci.depth, &ci.type, &ci.format,
			&ci.levels, &ci.layers, &img_data,
			&img_size, &free_data);
		break;
	case NE_TEXTURE_FORMAT_PNG:
		ret = asset_load_png(data, size, &ci.width, &ci.height,
			&ci.depth, &ci.type, &ci.format,
			&ci.levels, &ci.layers, &img_data,
			&img_size, &free_data);
		break;
	default:
		ret = NE_FORMAT_UNSUPPORTED;
		break;
	}

	if (ret != NE_OK)
		goto error;

	tex = gfx_create_texture(&ci, img_data, img_size);
	if (!tex)
		goto error;

	log_entry(TEXTURE_MODULE, LOG_DEBUG,
		"Loaded image from [%s]: w=%d, h=%d, d=%d, t=%d, f=%d, m=%d, l=%d, s=%llu",
		image_path, ci.width, ci.height, ci.depth, ci.type, ci.format,
		ci.levels, ci.layers, size);

	cJSON_Delete(json);
	json = NULL;
	free(metadata);
	metadata = NULL;
	free(data);
	data = NULL;
	if (free_data)
		free(img_data);

	return tex;

error:
	free(tex);
	free(data);
	free(metadata);
	io_close(file);
	cJSON_Delete(json);
	if (free_data)
		free(img_data);

	return NULL;
}

