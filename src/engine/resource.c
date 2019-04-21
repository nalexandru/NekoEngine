/* NekoEngine
 *
 * resource.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Resource Subsystem
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

#include <engine/resource.h>

#define RESOURCE_MODULE	"Resource"

typedef struct ne_res_info
{
	rt_string ri_path;
	rt_string ri_type;
	int32_t ri_references;
} ne_res_info;

typedef struct ne_res_procs
{
	uint64_t type_hash;
	res_load_proc load;
	res_unload_proc unload;
} ne_res_procs;

typedef struct ne_res
{
	struct ne_res_info res_info;
	void *res_ptr;
} ne_res;

static rt_array _resources, _res_procs;

static int
_res_proc_cmp_func(
	const void *item,
	const void *data)
{
	return !(((ne_res_procs *)item)->type_hash == *((uint64_t *)data));
}

void
_res_unload_real(ne_res *res)
{
	ne_res_procs *procs = rt_array_find(&_res_procs,
		&res->res_info.ri_type.hash, _res_proc_cmp_func);

	if (!procs) {
		log_entry(RESOURCE_MODULE, LOG_CRITICAL,
				"Handler not found for resource type [%s]",
				res->res_info.ri_type.data);
		return;
	}

	procs->unload(res->res_ptr);

	rt_string_release(&res->res_info.ri_path);
	rt_string_release(&res->res_info.ri_type);

	free(res);
}

ne_status
res_init(void)
{
	memset(&_resources, 0x0, sizeof(_resources));
	memset(&_res_procs, 0x0, sizeof(_res_procs));

	if (rt_array_init(&_resources, 10, sizeof(ne_res *)) < 0)
		return NE_FAIL;

	if (rt_array_init(&_res_procs, 10, sizeof(ne_res_procs)) < 0)
		return NE_FAIL;

	return NE_OK;
}

ne_status
res_register_type(
	const char *name,
	res_load_proc load_proc,
	res_unload_proc unload_proc)
{
	ne_res_procs procs;

	procs.type_hash = rt_hash_string(name);
	procs.load = load_proc;
	procs.unload = unload_proc;

	if (rt_array_find(&_res_procs,
		&procs.type_hash, _res_proc_cmp_func)) {
		log_entry(RESOURCE_MODULE, LOG_WARNING,
			"Attempt to register handler for [%s] multiple times", name);
		return NE_ALREADY_EXISTS;
	}

	return rt_array_add(&_res_procs, &procs);
}

void *
res_load(
	const char *path,
	const char *type)
{
	size_t i = 0;
	uint64_t hash = 0, type_hash = 0;
	ne_res *res = NULL;
	ne_res_procs *procs = NULL;

	if (!path || !type) {
		log_entry(RESOURCE_MODULE, LOG_DEBUG,
			"Invalid arguments for res_load [%s][%s]",
			path, type);
		return NULL;
	}

	hash = rt_hash_string(path);
	type_hash = rt_hash_string(type);

	for (i = 0; i < _resources.count; ++i) {
		res = rt_array_get_ptr(&_resources, i);

		if (res->res_info.ri_type.hash != type_hash ||
			res->res_info.ri_path.hash != hash)
			continue;

		++res->res_info.ri_references;
		return res->res_ptr;
	}

	if ((res = calloc(1, sizeof(*res))) == NULL) {
		return NULL;
	}

	procs = rt_array_find(&_res_procs,
		&type_hash, _res_proc_cmp_func);

	if (!procs) {
		log_entry(RESOURCE_MODULE, LOG_CRITICAL,
				"Handler not found for resource type [%s]", type);
		return NULL;
	}

	if ((res->res_ptr = procs->load(path)) == NULL) {
		log_entry(RESOURCE_MODULE, LOG_CRITICAL,
				"Failed to load resource [%s] of type [%s]", path, type);
		return NULL;
	}

	rt_string_init_with_cstr(&res->res_info.ri_path, path);
	rt_string_init_with_cstr(&res->res_info.ri_type, type);
	res->res_info.ri_references = 1;

	rt_array_add_ptr(&_resources, res);

	return res->res_ptr;
}

void
res_unload(
	const void *ptr,
	const char *type)
{
	size_t i = 0;
	uint64_t type_hash = 0;
	ne_res *res = NULL;

	if (!ptr)
		return;

	if (!type) {
		log_entry(RESOURCE_MODULE, LOG_DEBUG,
			"Invalid arguments for res_unload: no type specified");
		return;
	}

	type_hash = rt_hash_string(type);

	for (i = 0; i < _resources.count; ++i) {
		res = rt_array_get_ptr(&_resources, i);

		if (res->res_info.ri_type.hash != type_hash ||
			res->res_ptr != ptr) {
			res = NULL;
			continue;
		}

		break;
	}

	if (!res)
		return;

	if (--res->res_info.ri_references > 0)
		return;

	rt_array_remove(&_resources, i);
	_res_unload_real(res);
}

void
res_unload_all(void)
{
	size_t i = 0;
	ne_res *res = NULL;

	for (i = 0; i < _resources.count; ++i) {
		res = rt_array_get_ptr(&_resources, i);
		res->res_info.ri_references = 0;
		_res_unload_real(res);
	}

	_resources.count = 0;
}

void
res_release(void)
{
	res_unload_all();

	rt_array_release(&_res_procs);
	memset(&_res_procs, 0x0, sizeof(_res_procs));

	rt_array_release(&_resources);
	memset(&_resources, 0x0, sizeof(_resources));
}

