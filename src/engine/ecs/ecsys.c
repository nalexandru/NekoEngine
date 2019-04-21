/* NekoEngine
 *
 * ecsys.c
 * Author: Alexandru Naiman
 *
 * NekoEngine ECSystem
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

#include <system/log.h>
#include <system/compat.h>
#include <system/system.h>
#include <runtime/runtime.h>

#include <engine/task.h>
#include <engine/engine.h>
#include <ecs/ecsys.h>
#include <ecs/entity.h>
#include <ecs/component.h>

#define ECSYS_MODULE	"ECSYS"

struct worker_args
{
	struct ne_ec_system *sys;
	double dt;
	rt_array *array;
	uint32_t start;
	uint32_t count;
};

static rt_array _systems;
static rt_array _filtered_entities;

static void _ecsys_comp_update_proc(void *);
static void _ecsys_entity_update_proc(void *);

static int
_ecsys_insert_cmp(
	const void *item,
	const void *data)
{
	const struct ne_ec_system *sys = item;
	int32_t priority = *(int32_t *)data;

	if (priority > sys->priority)
		return 1;
	else if (priority < sys->priority)
		return -1;
	else
		return 0;
}

ne_status
ecsys_init(void)
{
	if (rt_array_init(&_systems, 40, sizeof(struct ne_ec_system)))
		return NE_FAIL;

	if (rt_array_init(&_filtered_entities, 100, sizeof(entity_handle)))
		return NE_FAIL;

	return NE_OK;
}

void
ecsys_release(void)
{
	for (size_t i = 0; i < _systems.count; ++i)
		free(((struct ne_ec_system *)
			rt_array_get(&_systems, i))->comp_types);

	rt_array_release(&_systems);
}

ne_status
ecsys_register(
	const char *name,
	const char *group,
	const comp_type_id *comp,
	size_t num_comp,
	ecsys_update_proc proc,
	bool parallel,
	int32_t priority)
{
	size_t pos = 0;
	struct ne_ec_system sys;

	memset(&sys, 0x0, sizeof(sys));

	// ensure we have room for the system
	if (!rt_array_create(&_systems))
		return NE_NO_MEMORY;
	--_systems.count;

	sys.name_hash = rt_hash_string(name);
	sys.group_hash = rt_hash_string(group);

	sys.comp_types = calloc(num_comp, sizeof(comp_type_id));
	if (!sys.comp_types)
		return NE_NO_MEMORY;

	memcpy(sys.comp_types, comp, sizeof(comp_type_id) * num_comp);
	sys.type_count = num_comp;
	sys.parallel = sys_cpu_count() > 1 ? parallel : false;
	sys.update = proc;
	sys.priority = priority;

	pos = rt_array_find_id(&_systems, &priority, _ecsys_insert_cmp);
	if (pos == RT_NOT_FOUND)
		pos = _systems.count;

	if (rt_array_insert(&_systems, &sys, pos) != SYS_OK) {
		free(sys.comp_types);
		return NE_FAIL;
	}

	return NE_OK;
}

static inline void
_serial_update_comp(
	struct ne_ec_system *sys,
	double dt,
	rt_array *components)
{
	void *ptr = NULL;
	for (size_t i = 0; i < components->count; ++i) {
		ptr = rt_array_get(components, i);
		sys->update(dt, &ptr);
	}
}

static inline void
_filter_entities(
	rt_array *ent,
	comp_type_id *comp_types,
	size_t type_count)
{
	rt_array *components;
	comp_type_id type = -1;
	size_t count = 0, min_count = SIZE_MAX;
	struct ne_comp_base *comp = NULL;
	bool valid = false;

	for (size_t i = 0; i < type_count; ++i) {
		count = comp_get_count(comp_types[i]);

		if (count >= min_count)
			continue;

		type = comp_types[i];
		min_count = count;
	}

	if (type == -1) {
		log_entry(ECSYS_MODULE, LOG_CRITICAL,
			"_filter_entities: Entity with least components not found. Is type_count set ?");
		return;
	}

	rt_array_clear(ent, false);
	components = comp_sys_get_all(type);

	if (ent->size < components->count)
		rt_array_resize(ent, components->count);

	for (size_t i = 0; i < components->count; ++i) {
		comp = rt_array_get(components, i);

		if (!comp->_owner)
			continue;

		valid = true;

		for (size_t j = 0; j < type_count; ++j) {
			if (entity_get_component(comp->_owner, comp_types[j]))
				continue;

			valid = false;
			break;
		}

		rt_array_add(ent, &comp->_owner);
	}
}


static inline void
_serial_update_ent(
	struct ne_ec_system *sys,
	double dt,
	rt_array *entities)
{
	entity_handle handle = 0;
	void *components[MAX_ENTITY_COMPONENTS];

	for (size_t i = 0; i < entities->count; ++i) {
		handle = *(entity_handle *)rt_array_get(entities, i);

		for (uint8_t j = 0; j < sys->type_count; ++j)
			components[j] = entity_get_component(handle,
				sys->comp_types[j]);

		sys->update(dt, components);
	}
}

static void
_ecsys_comp_update_proc(void *a)
{
	struct worker_args *args = a;
	uint32_t end = args->start + args->count;

//	log_entry("ECSYS", LOG_DEBUG, "update_proc: %d entities",
//			args->count);

	void *ptr = NULL;
	for (uint32_t i = args->start; i < end; ++i) {
//		log_entry("ECSYS", LOG_DEBUG, "update_proc: %d",
//			i);
		ptr = rt_array_get(args->array, i);
		args->sys->update(args->dt, &ptr);
	}
}

static void
_ecsys_entity_update_proc(void *a)
{
	struct worker_args *args = a;
	uint32_t end = args->start + args->count;
	entity_handle handle = 0;
	void *components[MAX_ENTITY_COMPONENTS];

	for (uint32_t i = args->start; i < end; ++i) {
		handle = *(entity_handle *)rt_array_get(args->array, i);

		for (uint8_t j = 0; j < args->sys->type_count; ++j)
			components[j] = entity_get_component(handle,
				args->sys->comp_types[j]);

		args->sys->update(args->dt, components);
	}
}

static inline void
_parallel_update(
	struct ne_ec_system *sys,
	double dt,
	rt_array *entities,
	void (*update_proc)(void *))
{
	uint32_t bucket_size = 0, num_buckets = task_num_workers(),
		 num_entities = entities->count, extra_entities = 0,
		 next_start = 0;

	if (num_entities <= num_buckets) {
		bucket_size = 1;
		num_buckets = entities->count;
	} else {
		bucket_size = num_entities / num_buckets;
		if (num_buckets * bucket_size != num_entities)
			extra_entities = num_entities - (num_buckets * bucket_size);
	}

	struct worker_args *args = calloc(num_buckets, sizeof(*args));
	assert(args);

	for (uint32_t i = 0; i < num_buckets; ++i) {
		args[i].sys = sys;
		args[i].dt = dt;
		args[i].array = entities;
		args[i].start = next_start;
		args[i].count = bucket_size;

		if (extra_entities) {
			++args[i].count;
			--extra_entities;
		}

		next_start += args[i].count;

		task_add(update_proc, &args[i]);
	}

	task_execute();
	free(args);
}

static INLINE void
_sys_update(
	struct ne_ec_system *sys,
	double dt)
{
	if (sys->type_count == 1) {
		if (sys->parallel)
			_parallel_update(sys, dt, comp_sys_get_all(sys->comp_types[0]),
				_ecsys_comp_update_proc);
		else
			_serial_update_comp(sys, dt,
				comp_sys_get_all(sys->comp_types[0]));
	} else {
		_filter_entities(&_filtered_entities,
			sys->comp_types, sys->type_count);

		if (sys->parallel)
			_parallel_update(sys, dt, &_filtered_entities,
				_ecsys_entity_update_proc);
		else
			_serial_update_ent(sys, dt, &_filtered_entities);
	}
}

void
ecsys_update_single(const char *name)
{
	uint64_t hash = rt_hash_string(name);
	double dt = engine_delta_time();

	for (size_t i = 0; i < _systems.count; ++i) {
		struct ne_ec_system *sys = rt_array_get(&_systems, i);
		if (sys->name_hash == hash) {
			_sys_update(rt_array_get(&_systems, i), dt);
			return;
		}
	}
}

void
ecsys_update_group(const char *name)
{
	uint64_t hash = rt_hash_string(name);
	double dt = engine_delta_time();

	for (size_t i = 0; i < _systems.count; ++i) {
		struct ne_ec_system *sys = rt_array_get(&_systems, i);
		if (sys->group_hash == hash)
			_sys_update(rt_array_get(&_systems, i), dt);
	}
}

void
ecsys_update_all(double dt)
{
	for (size_t i = 0; i < _systems.count; ++i)
		_sys_update(rt_array_get(&_systems, i), dt);
}

