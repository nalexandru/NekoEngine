/* NekoEngine
 *
 * input.c
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

#include <string.h>

#include <engine/input.h>
#include <engine/engine.h>

ne_status	sys_input_init(void);
void		sys_input_poll_controllers(void);
void		sys_input_release(void);

static uint8_t _key_state[256];
static uint8_t _prev_key_state[256];

uint8_t cursor_state = 0;

bool
input_cursor_visible(void)
{
	return cursor_state & CURSOR_VISIBLE;
}

bool
input_cursor_captured(void)
{
	return cursor_state & CURSOR_CAPTURED;
}

bool
input_get_key_down(ne_key code)
{
	return !_prev_key_state[code] ? _key_state[code] : false;
}

bool
input_get_key_up(ne_key code)
{
	return _prev_key_state[code] ? !_key_state[code] : false;
}

bool
input_get_key(ne_key code)
{
	return _key_state[code];
}

ne_status
input_init(void)
{
	memset(_key_state, 0x0, sizeof(_key_state));
	memset(_prev_key_state, 0x0, sizeof(_prev_key_state));

	return ne_headless ? NE_OK : sys_input_init();
}

void
input_key_event(
	ne_key code,
	bool state)
{
	_key_state[code] = state;
}

void
input_pre_update(void)
{
	sys_input_poll_controllers();
}

void
input_post_update(void)
{
	memmove(_prev_key_state, _key_state, sizeof(_key_state));
}

void
input_release(void)
{
	sys_input_release();
}

