/* NekoEngine
 *
 * material.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Material
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
#include <engine/resource.h>
#include <graphics/graphics.h>
#include <graphics/material.h>

#define MATERIAL_MODULE		"Material"

void *
load_material(const char *path)
{
	ne_file *file = NULL;
	struct ne_material *mat = NULL;
	char meta_path[PATH_MAX];
	char *metadata = NULL;
	cJSON *json = NULL;
	const cJSON *tmp = NULL;
	int64_t size = 0;

	if (!strstr(path, ".nemat"))
		snprintf(meta_path, PATH_MAX, "%s.nemat", path);
	else
		snprintf(meta_path, PATH_MAX, "%s", path);

	mat = calloc(1, sizeof(*mat));
	if (!mat) {
		log_entry(MATERIAL_MODULE, LOG_CRITICAL,
			"Failed to allocate memory");
		goto error;
	}

	mat->diffuse_map = ne_blank_texture;
	mat->roughness_map = ne_blank_texture;
	mat->metallic_map = ne_blank_texture;
	mat->ao_map = ne_blank_texture;
	mat->normal_map = ne_blank_normal_texture;

	kmVec4Fill(&mat->data.color, 1.f, 1.f, 1.f, 1.f);
	mat->data.metallic = 1.f;
	mat->data.roughness = 1.f;

	file = io_open(meta_path, IO_READ);
	if (!file) {
		log_entry(MATERIAL_MODULE, LOG_CRITICAL,
			"Failed to open file [%s]", meta_path);
		goto error;
	}

	metadata = io_read_text(file, &size);
	if (!metadata) {
		log_entry(MATERIAL_MODULE, LOG_CRITICAL,
			"Failed to read file [%s]", meta_path);
		goto error;
	}

	io_close(file);
	file = NULL;

	json = cJSON_Parse(metadata);
	if (!json) {
		log_entry(MATERIAL_MODULE, LOG_CRITICAL,
			"Failed to parse file [%s]: %s",
			meta_path, cJSON_GetErrorPtr());
		goto error;
	}

	if (!cJSON_IsObject(json)) {
		log_entry(MATERIAL_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No root object", meta_path);
		goto error;
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "color");
	if (cJSON_IsString(tmp) && tmp->valuestring) {
		if (meta_read_floats(tmp->valuestring, strlen(tmp->valuestring),
				(float *)&mat->data.color, 4) != NE_OK) {
			log_entry(MATERIAL_MODULE, LOG_CRITICAL,
				"Failed to load color for material [%s]",
				meta_path);
			goto error;
		}
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "roughness");
	if (cJSON_IsNumber(tmp) && tmp->valuestring)
		mat->data.roughness = (float)tmp->valuedouble;

	tmp = cJSON_GetObjectItemCaseSensitive(json, "metallic");
	if (cJSON_IsNumber(tmp) && tmp->valuestring)
		mat->data.metallic = (float)tmp->valuedouble;

	tmp = cJSON_GetObjectItemCaseSensitive(json, "transparent");
	if (cJSON_IsBool(tmp) && tmp->valuestring)
		mat->transparent = tmp->valueint;

	tmp = cJSON_GetObjectItemCaseSensitive(json, "diffuse_map");
	if (cJSON_IsString(tmp) && tmp->valuestring) {
		mat->diffuse_map = res_load(tmp->valuestring, RES_TEXTURE);
		if (!mat->diffuse_map) {
			log_entry(MATERIAL_MODULE, LOG_CRITICAL,
				"Failed to load diffuse map for material [%s]",
				meta_path);
			goto error;
		}
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "normal_map");
	if (cJSON_IsString(tmp) && tmp->valuestring) {
		mat->normal_map = res_load(tmp->valuestring, RES_TEXTURE);
		if (!mat->normal_map) {
			log_entry(MATERIAL_MODULE, LOG_CRITICAL,
				"Failed to load normal map for material [%s]",
				meta_path);
			goto error;
		}
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "metallic_map");
	if (cJSON_IsString(tmp) && tmp->valuestring) {
		mat->metallic_map = res_load(tmp->valuestring, RES_TEXTURE);
		if (!mat->metallic_map) {
			log_entry(MATERIAL_MODULE, LOG_CRITICAL,
				"Failed to load metallic map for material [%s]",
				meta_path);
			goto error;
		}
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "roughness_map");
	if (cJSON_IsString(tmp) && tmp->valuestring) {
		mat->roughness_map = res_load(tmp->valuestring, RES_TEXTURE);
		if (!mat->roughness_map) {
			log_entry(MATERIAL_MODULE, LOG_CRITICAL,
				"Failed to load roughness map for material [%s]",
				meta_path);
			goto error;
		}
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "ao_map");
	if (cJSON_IsString(tmp) && tmp->valuestring) {
		mat->ao_map = res_load(tmp->valuestring, RES_TEXTURE);
		if (!mat->ao_map) {
			log_entry(MATERIAL_MODULE, LOG_CRITICAL,
				"Failed to load ao map for material [%s]",
				meta_path);
			goto error;
		}
	}

	if (gfx_init_material(mat) != NE_OK)
		goto error;

	free(metadata);
	cJSON_Delete(json);

	return mat;

error:
	free(mat);
	free(metadata);
	io_close(file);
	cJSON_Delete(json);

	return NULL;
}

void
destroy_material(void *mat_ptr)
{
	struct ne_material *mat = mat_ptr;

	gfx_release_material(mat);

	if (mat->diffuse_map)
		res_unload(mat->diffuse_map, RES_TEXTURE);

	if (mat->normal_map)
		res_unload(mat->normal_map, RES_TEXTURE);

	if (mat->metallic_map)
		res_unload(mat->metallic_map, RES_TEXTURE);

	if (mat->roughness_map)
		res_unload(mat->roughness_map, RES_TEXTURE);

	if (mat->ao_map)
		res_unload(mat->ao_map, RES_TEXTURE);

	free(mat);
}

