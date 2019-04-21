/* NekoEngine
 *
 * ecsys_internal.h
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

#ifndef _NE_ECS_ECSYS_INTERNAL_H_
#define _NE_ECS_ECSYS_INTERNAL_H_

#include <stdint.h>

#include <runtime/array.h>

#include <ecs/ecsdefs.h>
#include <ecs/component.h>

struct ne_entity_comp
{
	comp_type_id type;
	comp_handle handle;
};

struct ne_entity
{
	size_t id;
	uint8_t comp_count;
	struct ne_entity_comp comp[MAX_ENTITY_COMPONENTS];
};

struct ne_entity_type
{
	uint64_t hash;
	uint8_t comp_count;
	comp_type_id comp_types[MAX_ENTITY_COMPONENTS];
};

struct ne_comp_handle
{
	comp_handle handle;
	comp_type_id type;
	size_t index;
};

struct ne_comp_base
{
	NE_COMPONENT;
};

struct ne_comp_type
{
	size_t size;
	uint64_t hash;
	comp_init_proc create;
	comp_release_proc destroy;
};

struct ne_ec_system
{
	ecsys_update_proc update;
	comp_type_id *comp_types;
	uint64_t name_hash;
	uint64_t group_hash;
	int32_t priority;
	size_t type_count;
	bool parallel;
};

typedef ne_status (*comp_sys_register_all_proc)(void);

ne_status	 comp_sys_init(void);
ne_status	 comp_sys_register(void);
void		 comp_sys_release(void);
rt_array	*comp_sys_get_all(comp_type_id type);

ne_status	 entity_sys_init(void);
void		 entity_sys_release(void);

ne_status	 ecsys_init(void);
void		 ecsys_update(double dt);
void		 ecsys_release(void);

#endif /* _NE_ECS_ECSYS_INTERNAL_H_ */
