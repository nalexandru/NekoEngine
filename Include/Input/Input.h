#ifndef _NE_INPUT_INPUT_H_
#define _NE_INPUT_INPUT_H_

#include <stdint.h>
#include <stdbool.h>

#include <Input/Codes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IN_MAX_CONTROLLERS	4

struct NeControllerState
{
	uint32_t buttons;
	float axis[CONTROLLER_AXIS_COUNT];
};

extern bool In_pointerVisible;
extern bool In_pointerCaptured;
extern bool In_buttonState[BTN_STATE_COUNT];
extern float In_mouseAxis[3];
extern float In_axisSensivity[255];
extern uint8_t In_connectedControllers;
extern struct NeControllerState In_controllerState[IN_MAX_CONTROLLERS];

bool In_InitInput(void);
void In_TermInput(void);

void In_Key(enum NeButton key, bool down);

void In_Update(void);

void In_EnableMouseAxis(bool enable);

enum NeAxis In_CreateVirtualAxis(const char *name, enum NeButton min, enum NeButton max);
enum NeAxis In_GetVirtualAxis(const char *name);

uint32_t In_CreateMap(const char *name);
void In_MapPrimaryButton(uint32_t map, enum NeButton btn, uint8_t controller);
void In_MapSecondaryButton(uint32_t map, enum NeButton btn, uint8_t controller);
void In_MapPrimaryAxis(uint32_t map, enum NeAxis axis, uint8_t controller);
void In_MapSecondaryAxis(uint32_t map, enum NeAxis axis, uint8_t controller);

bool In_Button(uint32_t map);
bool In_ButtonUp(uint32_t map);
bool In_ButtonDown(uint32_t map);
float In_Axis(uint32_t map);

bool In_UnmappedButton(enum NeButton btn, uint8_t controller);
bool In_UnmappedButtonUp(enum NeButton btn, uint8_t controller);
bool In_UnmappedButtonDown(enum NeButton btn, uint8_t controller);

float In_UnmappedAxis(enum NeAxis axis, uint8_t controller);

char In_KeycodeToChar(enum NeButton key, bool shift);

// Platform specific
bool In_SysInit(void);
void In_SysTerm(void);

void In_SysPollControllers(void);

void In_PointerPosition(uint16_t *x, uint16_t *y);
void In_SetPointerPosition(uint16_t x, uint16_t y);
void In_CapturePointer(bool capture);
void In_ShowPointer(bool show);

#ifdef __cplusplus
}
#endif

#endif /* _NE_INPUT_INPUT_H_ */

/* NekoEngine
 *
 * Input.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
 * -----------------------------------------------------------------------------
 */
