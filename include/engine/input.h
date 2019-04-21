/* NekoEngine
 *
 * input.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Input Subsystem
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

#ifndef _NE_ENGINE_INPUT_H_
#define _NE_ENGINE_INPUT_H_

#include <stdint.h>
#include <stdbool.h>

#include <engine/status.h>
#include <engine/keycodes.h>

#ifdef __cplusplus
extern "C" {
#endif

bool input_cursor_visible(void);
bool input_cursor_captured(void);

bool input_get_key_down(ne_key code);
bool input_get_key_up(ne_key code);
bool input_get_key(ne_key code);

void input_show_cursor(bool show);
void input_capture_cursor(bool capture);

#ifdef _NE_ENGINE_INTERNAL_
#define CURSOR_VISIBLE		0x00
#define CURSOR_CAPTURED		0x02

ne_status	input_init(void);

void		input_key_event(ne_key code, bool state);
void		input_pre_update(void);
void		input_post_update(void);
void		input_release(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* _NE_ENGINE_INPUT_H_ */

