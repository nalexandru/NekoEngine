/* NekoEngine
 *
 * guidefs.h
 * Author: Alexandru Naiman
 *
 * NekoEngine GUI Subsystem
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

#ifndef _NE_GUI_DEFS_H_
#define _NE_GUI_DEFS_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum ne_gui_evt
{
	GUI_EVT_CLICK = 0,
	GUI_EVT_MIDDLE_CLICK,
	GUI_EVT_RIGHT_CLICK,
	GUI_EVT_MOUSE_ENTER,
	GUI_EVT_MOUSE_LEAVE,
	GUI_EVT_MOUSE_MOVE,
	GUI_EVT_KEY_UP,
	GUI_EVT_KEY_DOWN,
	GUI_EVT_VALUE_CHANGED,

	NUM_GUI_EVT
} ne_gui_evt;

typedef struct ne_gui_key_evt
{
	uint32_t key_code;
} ne_gui_key_evt;

typedef struct ne_gui_mouse_evt
{
	uint32_t x, y;
} ne_gui_mouse_evt;

typedef struct ne_gui_value_changed_evt
{
	float value;
} ne_gui_value_changed_evt;

typedef struct ne_gui_evt_args
{
	union
	{
		ne_gui_key_evt *key;
		ne_gui_mouse_evt *mouse;
		ne_gui_value_changed_evt *value_changed;
	} evt;
} ne_gui_evt_args;

typedef void (*gui_handler)(ne_gui_evt_args *args);

#endif /* _NE_GUI_DEFS_H_ */

