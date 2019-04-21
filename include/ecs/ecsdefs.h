/* NekoEngine
 *
 * ecsdefs.h
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

#ifndef _NE_ECS_ECSDEFS_H_
#define _NE_ECS_ECSDEFS_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_ENTITY_COMPONENTS		30
#define NE_INVALID_COMPONENT		-1
#define NE_INVALID_COMPONENT_TYPE	-1
#define NE_INVALID_ENTITY		NULL
#define NE_INVALID_ECSYS		-1

typedef void * entity_handle;
typedef int64_t comp_handle;
typedef size_t comp_type_id;
typedef int64_t ecsys_handle;

typedef ne_status (*comp_init_proc)(void *, const void **);
typedef void (*comp_release_proc)(void *);

typedef void (*ecsys_update_proc)(double dt, void **comp);

#endif /* _NE_ECS_ECSDEFS_H_ */
