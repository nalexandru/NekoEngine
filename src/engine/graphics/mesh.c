/* NekoEngine
 *
 * mesh.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Mesh
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

#include <system/log.h>
#include <system/compat.h>

#include <engine/io.h>
#include <engine/json.h>
#include <engine/asset.h>
#include <engine/resource.h>
#include <graphics/mesh.h>
#include <graphics/vertex.h>
#include <graphics/graphics.h>
#include <graphics/primitive.h>

#define MESH_MODULE	"Mesh"

void *
load_mesh(const char *path)
{
	struct ne_mesh *mesh = NULL;
	ne_status ret;
	ne_file *file = NULL;
	uint8_t *data = NULL;
	int64_t size = 0;
	char *metadata = NULL;
	const char *mesh_path = NULL;
	cJSON *json = NULL;
	const cJSON *tmp = NULL;
	char meta_path[PATH_MAX];
	rt_array vertices, indices;

	if (!strstr(path, ".nemesh"))
		snprintf(meta_path, PATH_MAX, "%s.nemesh", path);
	else
		snprintf(meta_path, PATH_MAX, "%s", path);

	file = io_open(meta_path, IO_READ);
	if (!file) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
				"Failed to open file [%s]", meta_path);
		goto error;
	}

	metadata = io_read_text(file, &size);
	if (!metadata) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Failed to read file [%s]", meta_path);
		goto error;
	}

	io_close(file);
	file = NULL;

	mesh = calloc(1, sizeof(*mesh));
	if (!mesh) {
		log_entry(MESH_MODULE, LOG_CRITICAL, "Failed allocate memory");
		goto error;
	}

	json = cJSON_Parse(metadata);
	if (!json) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Failed to read file [%s]", meta_path);
		goto error;
	}

	if (!cJSON_IsObject(json)) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No root object", meta_path);
		goto error;
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "name");
	if (!cJSON_IsString(tmp) || !tmp->valuestring) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No mesh name", meta_path);
		goto error;
	}
	rt_string_init_with_cstr(&mesh->name, tmp->valuestring);

	tmp = cJSON_GetObjectItemCaseSensitive(json, "type");
	if (!cJSON_IsString(tmp) || !tmp->valuestring) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No mesh type", meta_path);
		goto error;
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "path");
	if (!cJSON_IsString(tmp) || !tmp->valuestring) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No mesh path", meta_path);
		goto error;
	}
	mesh_path = tmp->valuestring;

	tmp = cJSON_GetObjectItemCaseSensitive(json, "format");
	if (!cJSON_IsString(tmp) || !tmp->valuestring) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No mesh format", meta_path);
		goto error;
	}

	file = io_open(mesh_path, IO_READ);
       	if (!file) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
				"Failed to open file [%s]", path);
		goto error;
	}

	data = io_read_blob(file, &size);
	if (!data) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
				"Failed to read file [%s]", path);
		goto error;
	}

	if (!strncmp(tmp->valuestring, "NMESH2", strlen(tmp->valuestring))) {
		ret = asset_load_nmesh(data,
					size,
					&vertices,
					&indices,
					&mesh->groups);
	} else {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Unknown mesh file format [%s] for [%s]",
			tmp->valuestring, meta_path);
		goto error;
	}

	if (ret != NE_OK) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Failed to load mesh file [%s]",
			path);
		goto error;
	}

	struct ne_buffer_create_info bci;
	memset(&bci, 0x0, sizeof(bci));
	bci.access = NE_GPU_LOCAL;

	bci.size = rt_array_byte_size(&vertices);
	bci.usage = NE_VERTEX_BUFFER | NE_TRANSFER_DST;
	mesh->vtx_buffer = gfx_create_buffer(&bci, vertices.data, 0, bci.size);
	if (!mesh->vtx_buffer) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Failed to create vertex buffer");
		goto error;
	}

	bci.size = rt_array_byte_size(&indices);
	bci.usage = NE_INDEX_BUFFER | NE_TRANSFER_DST;
	mesh->idx_buffer = gfx_create_buffer(&bci, indices.data, 0, bci.size);
	if (!mesh->idx_buffer) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Failed to create vertex buffer");
		goto error;
	}

	log_entry(MESH_MODULE, LOG_DEBUG,
			"Loaded mesh from [%s] with %d vertices, %d indices, %d groups",
			path, vertices.count, indices.count, mesh->groups.count);

	rt_array_release(&vertices);
	rt_array_release(&indices);
	cJSON_Delete(json);
	free(metadata);
	free(data);

	return mesh;

error:
	rt_array_release(&vertices);
	rt_array_release(&indices);
	free(data);
	cJSON_Delete(json);
	if (mesh) {
		rt_string_release(&mesh->name);
		rt_array_release(&mesh->groups);
		free(mesh);
	}
	free(metadata);
	io_close(file);

	return NULL;
}

struct ne_mesh *
create_mesh(const char *name)
{
	struct ne_mesh *mesh = calloc(1, sizeof(*mesh));
	if (!mesh) {
		log_entry(MESH_MODULE, LOG_CRITICAL,
			"Failed to allocate memory");
		return NULL;
	}

	rt_string_init_with_cstr(&mesh->name, name);
	rt_array_init(&mesh->groups, 10, sizeof(struct ne_mesh_group));

	return mesh;
}

ne_status
upload_mesh(struct ne_mesh *mesh)
{
	return NE_OK;
}

void
destroy_mesh(void *mesh)
{
	if (!mesh)
		return;

	gfx_destroy_buffer(((struct ne_mesh *)mesh)->vtx_buffer);
	gfx_destroy_buffer(((struct ne_mesh *)mesh)->idx_buffer);

	rt_array_release(&((struct ne_mesh *)mesh)->groups);
	rt_string_release(&((struct ne_mesh *)mesh)->name);

	free(mesh);
}

// Drawable Component

ne_status
init_drawable_mesh_comp(void *c,
	const void **args)
{
	char *p;
	size_t len;
	struct ne_material *mat = NULL;
	struct ne_drawable_mesh_comp *comp = c;

	if (!c || !args)
		return NE_INVALID_ARGS;

	while (*args) {
		len = strlen(*args);

		if (!strncmp(*args, "mesh", len)) {
			const char *path = *(++args);

			if (strstr(path, "_builtin_")) {
				p = path + 9;
				len = strlen(p);

				if (!strncmp(p, "triangle", len))
					comp->mesh = gfx_primitive(PRIM_TRIANGLE);
				else if (!strncmp(p, "quad", len))
					comp->mesh = gfx_primitive(PRIM_QUAD);
				else if (!strncmp(p, "cube", len))
					comp->mesh = gfx_primitive(PRIM_CUBE);
				else if (!strncmp(p, "pyramid", len))
					comp->mesh = gfx_primitive(PRIM_PYRAMID);
				else if (!strncmp(p, "sphere", len))
					comp->mesh = gfx_primitive(PRIM_SPHERE);
				else if (!strncmp(p, "cone", len))
					comp->mesh = gfx_primitive(PRIM_CONE);
				else if (!strncmp(p, "cylinder", len))
					comp->mesh = gfx_primitive(PRIM_CYLINDER);
			} else {
				comp->mesh = res_load(path, RES_MESH);
			}

			if (!comp->mesh)
				goto error;

			if (rt_array_init_ptr(&comp->materials,
				comp->mesh->groups.count) != SYS_OK) {
				memset(&comp->materials, 0x0, sizeof(rt_array));
				goto error;
			}
		} else if (!strncmp(*args, "mat", len)) {
			mat = res_load(*(++args), RES_MATERIAL);
			if (!mat)
				goto error;

			// FIXME: Implement material instances
			void *mat_inst = malloc(sizeof(*mat));
			memcpy(mat_inst, mat, sizeof(*mat));

			rt_array_add_ptr(&comp->materials, mat_inst);
		}

		++args;
	}

	log_entry(MESH_MODULE, LOG_DEBUG,
		"Loaded MeshComponent with %d materials", comp->materials.count);

	//kmMat4Translation(&comp->data.model, 0.f, -4.f, 10.f);

	return gfx_init_drawable_mesh(comp);

error:
	if (!comp->mesh)
		return NE_FAIL;

	res_unload(comp->mesh, RES_MESH);

	rt_array_foreach_ptr(mat, &comp->materials)
		res_unload(mat, RES_MATERIAL);

	rt_array_release(&comp->materials);

	return NE_FAIL;
}

void
release_drawable_mesh_comp(void *c)
{
	struct ne_material *mat = NULL;
	struct ne_drawable_mesh_comp *comp = c;

	if (!c)
		return;

	gfx_release_drawable_mesh(comp);

	res_unload(comp->mesh, RES_MESH);

	rt_array_foreach_ptr(mat, &comp->materials)
		res_unload(mat, RES_MATERIAL);

	rt_array_release(&comp->materials);
}

