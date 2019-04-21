/* NekoEngine
 *
 * event.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Event 
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

#ifndef _NE_ENGINE_EVENT_H_
#define _NE_ENGINE_EVENT_H_

#include <stdint.h>

#include <engine/status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*evt_handler_proc)(uint64_t, void *);

uint64_t	evt_register_handler(int32_t evt, evt_handler_proc handler);
void		evt_unregister_handler(int32_t evt, uint64_t handler);
void		evt_broadcast(uint64_t evt, void *args);

#ifdef _NE_ENGINE_INTERNAL_
ne_status	evt_sys_init(void);
void		evt_sys_release(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _NE_ENGINE_EVENT_H_ */

