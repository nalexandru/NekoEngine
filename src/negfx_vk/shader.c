/* NekoEngine
 *
 * shader.c
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem Shader
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

#include <assert.h>
#include <stdlib.h>
#include <engine/io.h>
#include <engine/json.h>
#include <engine/status.h>
#include <engine/resource.h>

#include <vkgfx.h>
#include <debug.h>
#include <vkutil.h>
#include <shader.h>

#define SHADER_MODULE	"Shader"

void *
shmod_load(const char *path)
{
	VkResult res;
	VkShaderModuleCreateInfo ci;
	vkgfx_shader_module *mod = NULL;
	int64_t size = 0;
	uint32_t *data = NULL;
	ne_file *file = NULL;

	file = io_open(path, IO_READ);
	if (!file) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Failed to open file [%s]", path);
		goto error;
	}

	data = io_read_blob(file, &size);
	if (!data) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Failed to read file [%s]", path);
		goto error;
	}

	mod = calloc(1, sizeof(mod));
	if (!mod)
		goto error;

	memset(&ci, 0x0, sizeof(ci));
	ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ci.codeSize = size;
	ci.pCode = data;

	if ((res = vkCreateShaderModule(vkgfx_device, &ci, vkgfx_allocator,
		&mod->module)) != VK_SUCCESS) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Failed to create shader module for file [%s]: %s",
			path, vku_result_string(res));
		goto error;
	}

	VK_DBG_SET_OBJECT_NAME(mod->module,
		VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, path);

	free(data);

	return mod;

error:
	io_close(file);
	free(mod);
	free(data);

	return NULL;
}

void
shmod_destroy(void *res)
{
	vkgfx_shader_module *mod = res;

	if (!mod || mod->module == VK_NULL_HANDLE)
		return;

	vkDestroyShaderModule(vkgfx_device, mod->module, vkgfx_allocator);
	free(mod);
}

static inline VkShaderStageFlagBits
_string_to_stage(
	const char *str,
	size_t len)
{
	if (!strncmp(str, "vertex", len))
		return VK_SHADER_STAGE_VERTEX_BIT;
	else if (!strncmp(str, "fragment", len))
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	else if (!strncmp(str, "compute", len))
		return VK_SHADER_STAGE_COMPUTE_BIT;
	else if (!strncmp(str, "tessellation_control", len))
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	else if (!strncmp(str, "tessellation_evaluation", len))
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	else if (!strncmp(str, "geometry", len))
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	else if (!strncmp(str, "raygen", len))
		return VK_SHADER_STAGE_RAYGEN_BIT_NV;
	else if (!strncmp(str, "any_hit", len))
		return VK_SHADER_STAGE_ANY_HIT_BIT_NV;
	else if (!strncmp(str, "closest_hit", len))
		return VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	else if (!strncmp(str, "miss", len))
		return VK_SHADER_STAGE_MISS_BIT_NV;
	else if (!strncmp(str, "intersection", len))
		return VK_SHADER_STAGE_INTERSECTION_BIT_NV;
	else if (!strncmp(str, "callable", len))
		return VK_SHADER_STAGE_CALLABLE_BIT_NV;
	else if (!strncmp(str, "task", len))
		return VK_SHADER_STAGE_TASK_BIT_NV;
	else if (!strncmp(str, "mesh", len))
		return VK_SHADER_STAGE_MESH_BIT_NV;

	return -1;
}

static inline VkDescriptorType
_string_to_descriptor_type(
	const char *str,
	size_t len)
{
	if (!strncmp(str, "image_sampler", len))
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	else if (!strncmp(str, "sampler", len))
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	else if (!strncmp(str, "sampled_image", len))
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	else if (!strncmp(str, "storage_image", len))
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	else if (!strncmp(str, "uniform_texel_buffer", len))
		return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	else if (!strncmp(str, "storage_texel_buffer", len))
		return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	else if (!strncmp(str, "uniform_buffer", len))
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	else if (!strncmp(str, "storage_buffer", len))
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	else if (!strncmp(str, "dynamic_uniform_buffer", len))
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	else if (!strncmp(str, "dynamic_storage_buffer", len))
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	else if (!strncmp(str, "input_attachment", len))
		return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	else if (!strncmp(str, "inline_uniform_block", len))
		return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
	else if (!strncmp(str, "acceleration_structure", len))
		return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

	return -1;
}

static inline bool
_read_stage(
	const cJSON *json,
	vkgfx_shader *sh,
	int num,
	const char *path)
{
	const cJSON *st_type =
		cJSON_GetObjectItemCaseSensitive(json, "type");
	const cJSON *st_module =
		cJSON_GetObjectItemCaseSensitive(json, "module");
	const cJSON *st_spec =
		cJSON_GetObjectItemCaseSensitive(json, "specialization");

	if (!cJSON_IsString(st_type) || !st_type->valuestring) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No stage type", path);
		return false;
	}

	if (!cJSON_IsString(st_module) || !st_module->valuestring) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No stage", path);
		return false;
	}

	const cJSON *spec;
	cJSON_ArrayForEach(spec, st_spec) {
		const cJSON *sp_id =
			cJSON_GetObjectItemCaseSensitive(spec, "id");
		const cJSON *sp_value =
			cJSON_GetObjectItemCaseSensitive(spec, "value");

		if (!cJSON_IsNumber(sp_id)) {
			log_entry(SHADER_MODULE, LOG_CRITICAL,
				"Invalid JSON file [%s]: No sp id",
				path);
			return false;
		}

		if (!cJSON_IsNumber(sp_value)) {
			log_entry(SHADER_MODULE, LOG_CRITICAL,
				"Invalid JSON file [%s]: No sp value",
				path);
			return false;
		}

		uint32_t sc = sh->stages[num].sc_count++;
		sh->stages[num].sp_const[sc].id = sp_id->valueint;
		sh->stages[num].sp_const[sc].value = sp_value->valueint;
	}

	sh->stages[num].stage = _string_to_stage(st_type->valuestring,
			strlen(st_type->valuestring));

	sh->stages[num].sh_mod = res_load(st_module->valuestring,
						RES_SHADER_MODULE);
	if (!sh->stages[num].sh_mod)
		return false;

	return true;
}

static inline VkDescriptorSetLayout
_read_layout(
	const cJSON *json,
	const char *path)
{
	const cJSON *tmp = NULL;
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	VkDescriptorSetLayoutCreateInfo ci;
	memset(&ci, 0x0, sizeof(ci));

	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.bindingCount = cJSON_GetArraySize(json);

	uint8_t b_num = 0;
	VkDescriptorSetLayoutBinding *bindings = calloc(ci.bindingCount,
			sizeof(VkDescriptorSetLayoutBinding));
	assert(bindings);
	ci.pBindings = bindings;

	cJSON_ArrayForEach(tmp, json) {
		const cJSON *b_type =
			cJSON_GetObjectItemCaseSensitive(tmp, "type");
		const cJSON *b_bind =
			cJSON_GetObjectItemCaseSensitive(tmp, "binding");
		const cJSON *b_count =
			cJSON_GetObjectItemCaseSensitive(tmp, "count");
		const cJSON *b_stages =
			cJSON_GetObjectItemCaseSensitive(tmp, "stages");

		if (!cJSON_IsString(b_type) || !b_type->valuestring) {
			log_entry(SHADER_MODULE, LOG_CRITICAL,
				"Invalid JSON file [%s]: No bind type",
				path);
			goto exit;
		}
		bindings[b_num].descriptorType =
			_string_to_descriptor_type(b_type->valuestring,
				strlen(b_type->valuestring));

		if (!cJSON_IsNumber(b_bind)) {
			log_entry(SHADER_MODULE, LOG_CRITICAL,
				"Invalid JSON file [%s]: No bind binding",
				path);
			goto exit;
		}
		bindings[b_num].binding = b_bind->valueint;

		if (!cJSON_IsNumber(b_count)) {
			log_entry(SHADER_MODULE, LOG_CRITICAL,
				"Invalid JSON file [%s]: No bind count",
				path);
			goto exit;
		}
		bindings[b_num].descriptorCount = b_count->valueint;

		const cJSON *b_st = NULL;
		if (!cJSON_IsArray(b_stages)) {
			log_entry(SHADER_MODULE, LOG_CRITICAL,
				"Invalid JSON file [%s]: No bind stages", path);
			goto exit;
		}
		cJSON_ArrayForEach(b_st, b_stages) {
			if (!cJSON_IsString(b_st) || !b_st->valuestring) {
				log_entry(SHADER_MODULE, LOG_CRITICAL,
					"Invalid JSON file [%s]: Bind stage",
					path);
				goto exit;
			}

			bindings[b_num].stageFlags |=
				_string_to_stage(b_st->valuestring,
					strlen(b_st->valuestring));
		}

		++b_num;
	}

	VkResult res = vkCreateDescriptorSetLayout(vkgfx_device,
			&ci, vkgfx_allocator, &layout);
	if (res != VK_SUCCESS)
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Failed to create descriptor set layout for [%s]: %s",
			path, vku_result_string(res));

exit:
	free(bindings);
	return layout;
}

static inline bool
_read_pconst(
	const cJSON *json,
	const char *path,
	VkPushConstantRange *range)
{
	VkShaderStageFlags flags = 0;

	const cJSON *pc_stages =
		cJSON_GetObjectItemCaseSensitive(json, "stages");
	const cJSON *pc_offset =
		cJSON_GetObjectItemCaseSensitive(json, "offset");
	const cJSON *pc_size =
		cJSON_GetObjectItemCaseSensitive(json, "size");

	if (!cJSON_IsNumber(pc_offset)) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No p const offset",
			path);
		return false;
	}

	if (!cJSON_IsNumber(pc_size)) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No p const size",
			path);
		return false;
	}

	const cJSON *pc_st = NULL;
	if (!cJSON_IsArray(pc_stages)) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No p const stages",
			path);
		false;
	}

	cJSON_ArrayForEach(pc_st, pc_stages)
	{
		if (!cJSON_IsString(pc_st) || !pc_st->valuestring) {
			log_entry(SHADER_MODULE, LOG_CRITICAL,
				"Invalid JSON file [%s]: P const stage",
				path);
			return false;
		}

		flags |= _string_to_stage(pc_st->valuestring,
				strlen(pc_st->valuestring));
	}

	range->stageFlags = flags;
	range->offset = pc_offset->valueint;
	range->size = pc_size->valueint;

	return true;
}

void *
sh_load(const char *path)
{
	vkgfx_shader *sh = NULL;
	int64_t size = 0;
	char *text = NULL;
	ne_file *file = NULL;
	int ret = 0;
	cJSON *json = NULL;
	const cJSON *tmp = NULL, *tmp_arr = NULL;
	size_t len = 0;

	file = io_open(path, IO_READ);
	if (!file) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Failed to open file [%s]", path);
		goto error;
	}

	text = io_read_text(file, &size);
	if (!text) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Failed to read file [%s]", path);
		goto error;
	}

	json = cJSON_Parse(text);
	if (!json) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Failed to parse file [%s]: %s",
			path, cJSON_GetErrorPtr());
		goto error;
	}

	if (!cJSON_IsObject(json)) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No root object", path);
		goto error;
	}

	tmp = cJSON_GetObjectItemCaseSensitive(json, "name");
	tmp = cJSON_GetObjectItemCaseSensitive(json, "type");
	if (!cJSON_IsString(tmp) || !tmp->valuestring) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No shader type", path);
		goto error;
	}

	sh = calloc(1, sizeof(*sh));
	if (!sh)
		goto error;

	len = strlen(tmp->valuestring);
	if (!strncmp(tmp->valuestring, "graphics", len)) {
		sh->type = VKGFX_GRAPHICS;
	} else if (!strncmp(tmp->valuestring, "compute", len)) {
		sh->type = VKGFX_COMPUTE;
	} else if (!strncmp(tmp->valuestring, "raytrace", len)) {
		sh->type = VKGFX_RAYTRACE;
	} else {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: Unknown shader type", path);
		goto error;
	}

	tmp_arr = cJSON_GetObjectItemCaseSensitive(json, "stages");
	if (!cJSON_IsArray(tmp_arr)) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No stages", path);
		goto error;
	}
	sh->stage_count = (uint8_t)cJSON_GetArraySize(tmp_arr);
	int st_num = 0;
	cJSON_ArrayForEach(tmp, tmp_arr) {
		if (!_read_stage(tmp, sh, st_num++, path))
			goto error;
	}

	tmp_arr = cJSON_GetObjectItemCaseSensitive(json, "layouts");
	if (!cJSON_IsArray(tmp_arr)) {
		log_entry(SHADER_MODULE, LOG_CRITICAL,
			"Invalid JSON file [%s]: No layouts", path);
		goto error;
	}
	cJSON_ArrayForEach(tmp, tmp_arr) {
		sh->layouts[sh->layout_count] = _read_layout(tmp, path);
		if (sh->layouts[sh->layout_count] == VK_NULL_HANDLE)
			goto error;
		++sh->layout_count;
	}

	tmp_arr = cJSON_GetObjectItemCaseSensitive(json, "push_constants");
	if (cJSON_IsArray(tmp_arr)) {
		cJSON_ArrayForEach(tmp, tmp_arr)
		{
			if (!_read_pconst(tmp, path,
				&sh->pconst_ranges[sh->pconst_count]))
				goto error;
			++sh->pconst_count;
		}
	}

	cJSON_Delete(json);
	free(text);
	text = NULL;

	return sh;

error:
	cJSON_Delete(json);
	io_close(file);
	free(sh);
	free(text);

	return NULL;
}

void
sh_destroy(void *res)
{
	vkgfx_shader *sh = res;

	for (uint8_t i = 0; i < sh->layout_count; ++i)
		vkDestroyDescriptorSetLayout(vkgfx_device,
			sh->layouts[i], vkgfx_allocator);

	for (uint8_t i = 0; i < VKGFX_STAGE_COUNT; ++i)
		if (sh->stages[i].sh_mod != VK_NULL_HANDLE)
			res_unload(sh->stages[i].sh_mod, RES_SHADER_MODULE);

	free(res);
}
