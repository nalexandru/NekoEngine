/* NekoEngine
 *
 * component.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Component
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

#ifndef _NE_ECS_COMPONENT_H_
#define _NE_ECS_COMPONENT_H_

#include <stdint.h>

#include <engine/status.h>
#include <ecs/ecsdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NE_COMPONENT		\
	void *_self;		\
	entity_handle _owner

comp_handle	 comp_create(const char *type_name, entity_handle owner, const void **args);
comp_handle	 comp_create_id(comp_type_id id, entity_handle owner, const void **args);
void		 comp_destroy(comp_handle comp);
void		*comp_ptr(comp_handle comp);
comp_type_id	 comp_get_type(comp_handle comp);
comp_type_id	 comp_get_type_id(const char *type_name);
size_t		 comp_get_count(comp_type_id type);
entity_handle	 comp_get_owner(comp_handle comp);
void		 comp_set_owner(comp_handle comp, entity_handle owner);
ne_status	 comp_register(const char *name, size_t size,
			comp_init_proc init, comp_release_proc release);

#define NE_REGISTER_COMPONENT(name, type, init_proc, release_proc)

#ifdef __cplusplus
}
#endif

#endif /* _NE_ECS_COMPONENT_H_ */
