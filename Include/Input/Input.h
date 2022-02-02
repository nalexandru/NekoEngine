#ifndef _NE_INPUT_INPUT_H_
#define _NE_INPUT_INPUT_H_

#include <stdint.h>
#include <stdbool.h>

#include <Input/Codes.h>

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

#endif /* _NE_INPUT_INPUT_H_ */
