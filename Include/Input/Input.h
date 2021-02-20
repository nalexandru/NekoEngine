#ifndef _IN_INPUT_H_
#define _IN_INPUT_H_

#include <stdint.h>
#include <stdbool.h>

#include <Input/Codes.h>

#include <wchar.h>

#define IN_MAX_CONTROLLERS	4

struct ControllerState
{
	uint32_t buttons;
	float axis[CONTROLLER_AXIS_COUNT];
};

extern bool In_pointerVisible;
extern bool In_pointerCaptured;
extern bool In_buttonState[BTN_STATE_COUNT];
extern float In_mouseAxis[3];
extern uint8_t In_connectedControllers;
extern struct ControllerState In_controllerState[IN_MAX_CONTROLLERS];

bool In_InitInput(void);
void In_TermInput(void);

void In_Update(void);

enum Axis In_CreateVirtualAxis(const wchar_t *name, enum Button min, enum Button max);
enum Axis In_GetVirtualAxis(const wchar_t *name);

uint32_t In_CreateMap(const wchar_t *name);
void In_MapPrimaryButton(uint32_t map, enum Button btn, uint8_t controller);
void In_MapSecondaryButton(uint32_t map, enum Button btn, uint8_t controller);
void In_MapPrimaryAxis(uint32_t map, enum Axis axis, uint8_t controller);
void In_MapSecondaryAxis(uint32_t map, enum Axis axis, uint8_t controller);

bool In_Button(uint32_t map);
bool In_ButtonUp(uint32_t map);
bool In_ButtonDown(uint32_t map);
float In_Axis(uint32_t map);

bool In_UnmappedButton(enum Button btn, uint8_t controller);
bool In_UnmappedButtonUp(enum Button btn, uint8_t controller);
bool In_UnmappedButtonDown(enum Button btn, uint8_t controller);

float In_UnmappedAxis(enum Axis axis, uint8_t controller);

// Platform specific
bool In_SysInit(void);
void In_SysTerm(void);

void In_SysPollControllers(void);

void In_PointerPosition(uint16_t *x, uint16_t *y);
void In_SetPointerPosition(uint16_t x, uint16_t y);
void In_CapturePointer(bool capture);
void In_ShowPointer(bool show);

#endif /* _IN_INPUT_H_ */
