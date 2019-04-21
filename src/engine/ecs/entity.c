/* NekoEngine
 *
 * entity.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Entity
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
#include <runtime/runtime.h>

#include <ecs/entity.h>
#include <ecs/ecsys_internal.h>

#define ENTITY_MODULE	"Entity"

static rt_array _entity_types;
static rt_array _entities;

int
_entity_type_cmp(
	const void *item,
	const void *data)
{
	return !(((struct ne_entity_type *)item)->hash == *((uint64_t *)data));
}

static inline bool
_entity_add_component(
	struct ne_entity *ent,
	comp_type_id type,
	comp_handle handle)
{
	struct ne_entity_comp *comp = NULL;

	for (uint8_t i = 0; i < ent->comp_count; ++i)
		if (ent->comp[i].type == type)
			return false;

	comp = &ent->comp[ent->comp_count++];
	comp->type = type;
	comp->handle = handle;
	comp_set_owner(handle, ent);

	return true;
}

static inline bool
_entity_create_component(
	struct ne_entity *ent,
	comp_type_id type,
	const void **args)
{
	comp_handle handle = comp_create_id(type, ent, args);

	if (handle == NE_INVALID_COMPONENT)
		return false;

	if (!_entity_add_component(ent, type, handle)) {
		log_entry(ENTITY_MODULE, LOG_CRITICAL,
			"Failed to add component of type [%d]", type);
		return false;
	}

	return true;
}

ne_status
entity_sys_init(void)
{
	memset(&_entities, 0x0, sizeof(_entities));
	memset(&_entity_types, 0x0, sizeof(_entity_types));

	if (rt_array_init(&_entities, 100, sizeof(struct ne_entity *)))
		return NE_FAIL;

	if (rt_array_init(&_entity_types, 10, sizeof(struct ne_entity_type)))
		return NE_FAIL;

	return NE_OK;
}

void
entity_sys_release(void)
{
	for (size_t i = 0; i < _entities.count; ++i)
		free(rt_array_get_ptr(&_entities, i));

	rt_array_release(&_entity_types);
	rt_array_release(&_entities);
}

entity_handle
entity_create(const char *type_name)
{
	size_t id = 0;
	uint64_t hash = 0;
	struct ne_entity *ent = NULL;
	struct ne_entity_type *type = NULL;

	if (type_name) {
		hash = rt_hash_string(type_name);
		type = rt_array_find(&_entity_types, &hash, _entity_type_cmp);
		ent = entity_create_args(type->comp_types, NULL, type->comp_count);
	} else {
		ent = calloc(1, sizeof(struct ne_entity));
	}

	if (!ent)
		return NE_INVALID_ENTITY;

	if (rt_array_add_ptr(&_entities, ent) != SYS_OK) {
		free(ent);
		return NE_INVALID_ENTITY;
	}

	return ent;
}

entity_handle
entity_create_args(
	const comp_type_id *comp_types,
	const void ***comp_args,
	uint8_t count)
{
	struct ne_entity *ent = NULL;
	struct ne_entity_comp *comp = NULL;

	if (count > MAX_ENTITY_COMPONENTS)
		return NE_INVALID_ENTITY;

	ent = calloc(1, sizeof(struct ne_entity));
	if (!ent)
		return NE_INVALID_ENTITY;

	for (uint8_t i = 0; i < count; ++i) {
		if (!_entity_create_component(ent, comp_types[i],
				comp_args ? comp_args[i] : NULL)) {
			free(ent);
			return NE_INVALID_ENTITY;
		}
	}

	if (rt_array_add_ptr(&_entities, ent) != SYS_OK) {
		free(ent);
		return NE_INVALID_ENTITY;
	}

	return ent;
}

entity_handle
entity_create_v(
	int count,
	const struct ne_entity_comp_info *info)
{
	va_list va;
	struct ne_entity *ent = NULL;

	if (count > MAX_ENTITY_COMPONENTS)
		return NE_INVALID_ENTITY;

	ent = calloc(1, sizeof(struct ne_entity));
	if (!ent)
		return NE_INVALID_ENTITY;

	for (uint8_t i = 0; i < count; ++i) {
		if (!_entity_create_component(ent,
				comp_get_type_id(info[i].type), info[i].args)) {
			free(ent);
			return NE_INVALID_ENTITY;
		}
	}

	if (rt_array_add_ptr(&_entities, ent) != SYS_OK) {
		free(ent);
		return NE_INVALID_ENTITY;
	}

	return ent;
}

entity_handle
entity_create_with_comp(int count, ...)
{
	va_list va;
	struct ne_entity *ent = NULL;
	comp_handle handle;

	if (count > MAX_ENTITY_COMPONENTS)
		return NE_INVALID_ENTITY;

	ent = calloc(1, sizeof(struct ne_entity));
	if (!ent)
		return NE_INVALID_ENTITY;

	va_start(va, count);
	for (uint8_t i = 0; i < count; ++i) {
		handle = va_arg(va, comp_handle);

		if (!_entity_add_component(ent, handle, comp_get_type(handle))) {
			free(ent);
			return NE_INVALID_ENTITY;
		}
	}

	if (rt_array_add_ptr(&_entities, ent) != SYS_OK) {
		free(ent);
		return NE_INVALID_ENTITY;
	}

	for (uint8_t i = 0; i < ent->comp_count; ++i)
		comp_set_owner(ent->comp[i].handle, ent);

	return ent;
}

bool
entity_add_new_component(
	entity_handle ent,
	comp_type_id type,
	const void **args)
{
	return _entity_create_component(ent, type, args);
}

bool
entity_add_component(
	entity_handle ent,
	comp_type_id type,
	comp_handle comp)
{
	return _entity_add_component(ent, type, comp);
}

void *
entity_get_component(
	entity_handle handle,
	comp_type_id type)
{
	struct ne_entity *ent = handle;

	for (uint8_t i = 0; i < ent->comp_count; ++i)
		if (ent->comp[i].type == type)
			return comp_ptr(ent->comp[i].handle);

	return NULL;
}

void
entity_remove_component(
	entity_handle handle,
	comp_type_id type)
{
	struct ne_entity *ent = handle;
	uint8_t id = 0;

	for (id = 0; id < ent->comp_count; ++id)
		if (ent->comp[id].type == type)
			break;

	comp_destroy(ent->comp[id].handle);
	--ent->comp_count;

	if (id == ent->comp_count)
		return;

	memcpy(&ent->comp[id], &ent->comp_count, sizeof(struct ne_entity_comp));
}

void
entity_destroy(entity_handle handle)
{
	struct ne_entity *ent = handle;
	size_t dst_id = 0;

	for (uint8_t i = 0; i < ent->comp_count; ++i)
		comp_destroy(ent->comp[i].handle);

	dst_id = ent->id;
	free(ent);

	memcpy(rt_array_get(&_entities, dst_id),
		rt_array_last(&_entities), sizeof(void *));

	ent = rt_array_get_ptr(&_entities, dst_id);
	ent->id = dst_id;
	--_entities.count;
}

ne_status
entity_register(
	const char *type_name,
	const comp_type_id *comp_types,
	uint8_t type_count)
{
	struct ne_entity_type type;

	if (type_count > MAX_ENTITY_COMPONENTS)
		return NE_TOO_MANY_COMPONENTS;

	type.hash = rt_hash_string(type_name);
	type.comp_count = type_count;
	memcpy(type.comp_types, comp_types, sizeof(comp_type_id) * type_count);

	return rt_array_add(&_entity_types, &type) == SYS_OK ? NE_OK : NE_FAIL;
}

void *
entity_ptr(entity_handle ent)
{
	return rt_array_get_ptr(&_entities,
		((struct ne_entity *)ent)->id);
}

