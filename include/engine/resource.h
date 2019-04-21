/* NekoEngine
 *
 * resource.h
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


#ifndef _NE_ENGINE_RESOURCE_H_
#define _NE_ENGINE_RESOURCE_H_

#include <stdint.h>

#include <runtime/string.h>

#include <engine/status.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RES_TEXTURE		"res_texture"
#define RES_MESH		"res_mesh"
#define RES_SKELETAL_MESH	"res_skeletal_mesh"
#define RES_SOUND		"res_sound"
#define RES_ANIMATION		"res_animation"
#define RES_MATERIAL		"res_material"
#define RES_BLOB		"res_blob"
#define RES_FONT		"res_font"

typedef void *(*res_load_proc)(const char *path);
typedef void (*res_unload_proc)(void *res);

ne_status	 res_register_type(const char *name, res_load_proc load_proc, res_unload_proc unload_proc);
void	 	*res_load(const char *path, const char *type);
void		 res_unload(const void *ptr, const char *type);

#ifdef _NE_ENGINE_INTERNAL_

ne_status	res_init(void);
void		res_unload_all(void);
void		res_release(void);

#endif /* _NE_ENGINE_INTERNAL */

#ifdef __cplusplus
}
#endif

#endif /* _NE_ENGINE_RESOURCE_H_ */

