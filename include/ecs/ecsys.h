/* NekoEngine
 *
 * ecsys.h
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

#ifndef _NE_ECS_ECSYS_H_
#define _NE_ECS_ECSYS_H_

#include <stdint.h>

#include <engine/status.h>
#include <ecs/ecsdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define		ECSYS_GROUP_MANUAL		"manual"
#define		ECSYS_GROUP_PRE_LOGIC	"grp_pre_logic"
#define		ECSYS_GROUP_LOGIC	"grp_logic"
#define		ECSYS_GROUP_PRE_RENDER	"grp_pre_render"
#define		ECSYS_GROUP_RENDER	"grp_render"

#define		ECSYS_PRI_CULLING	-20000
#define		ECSYS_PRI_CAM_VIEW	-25000

ne_status 	ecsys_register(const char *name, const char *group, const comp_type_id *comp, size_t num_comp, ecsys_update_proc proc, bool parallel, int32_t priority);

void		ecsys_update_single(const char *name);
void		ecsys_update_group(const char *name);

#ifdef _NE_ENGINE_INTERNAL_

#include <ecs/ecsys_internal.h>

#endif /* _NE_ENGINE_INTERNAL_ */

#ifdef __cplusplus
}
#endif

#endif /* _NE_ECS_ECSYS_H_ */
