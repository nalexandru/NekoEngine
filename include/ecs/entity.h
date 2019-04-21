/* NekoEngine
 *
 * entity.h
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

#ifndef _NE_ECS_ENTITY_H_
#define _NE_ECS_ENTITY_H_

#include <stdint.h>
#include <stdarg.h>

#include <runtime/array.h>

#include <engine/status.h>
#include <ecs/ecsdefs.h>
#include <ecs/component.h>

struct ne_entity_comp_info
{
	const char *type;
	const void **args;
};

entity_handle	 entity_create(const char *name);
entity_handle	 entity_create_args(const comp_type_id *comp_types, const void ***comp_args, uint8_t type_count);
entity_handle	 entity_create_v(int count, const struct ne_entity_comp_info *info);
bool		 entity_add_component(entity_handle ent, comp_type_id type, comp_handle comp);
bool		 entity_add_new_component(entity_handle ent, comp_type_id type, const void **args);
void		*entity_get_component(entity_handle ent, comp_type_id type);
void		 entity_remove_component(entity_handle ent, comp_type_id type);
void		 entity_destroy(entity_handle ent);
ne_status	 entity_register(const char *name, const comp_type_id *comp_types, uint8_t type_count);
void		*entity_ptr(entity_handle ent);

#endif /* _NE_ECS_ENTITY_H_ */
