#include <Input/Input.h>
#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <Runtime/Runtime.h>
#include <System/Memory.h>

#define BUFF_SZ		64

#define CFG_INIT	0
#define CFG_VA		1
#define CFG_AM		2
#define CFG_KM		3

struct KeyAxis
{
	enum Button max;
	enum Button min;
	uint64_t hash;
};

struct Map
{
	union {
		struct {
			enum Button primary;
			uint8_t primaryDevice;
			enum Button secondary;
			uint8_t secondaryDevice;
		} key;
		struct {
			enum Axis primary;
			uint8_t primaryDevice;
			enum Axis secondary;
			uint8_t secondaryDevice;
		} axis;
	};
	uint64_t hash;
};

bool In_pointerVisible = true;
bool In_pointerCaptured = false;
bool In_buttonState[BTN_STATE_COUNT];
float In_mouseAxis[3] = { 0.f, 0.f, 0.f };
uint8_t In_connectedControllers = 0;
struct ControllerState In_controllerState[IN_MAX_CONTROLLERS];

static bool _enableMouseAxis = false;
static bool _prevButtonState[BTN_STATE_COUNT];
static struct ControllerState _prevControllerState[IN_MAX_CONTROLLERS];
static struct Array _map;
static struct Array _virtualAxis;

#define GPAD_BTN(x, state) (state.buttons & (1 << (x - BTN_GPAD_BTN_BASE)))

bool
In_InitInput(void)
{
	char *buff = NULL;
	wchar_t *wbuff = NULL;
	struct Stream stm;
	uint8_t mode = CFG_INIT;

	memset(In_buttonState, 0x0, sizeof(In_buttonState));
	memset(In_controllerState, 0x0, sizeof(In_controllerState));

	if (!In_SysInit())
		return false;

	if (!E_FileStream("/Config/Input.ini", IO_READ, &stm))
		return true;

	buff = Sys_Alloc(sizeof(char), BUFF_SZ, MH_Transient);
	wbuff = Sys_Alloc(sizeof(wchar_t), BUFF_SZ, MH_Transient);

	while (E_ReadStreamLine(&stm, buff, BUFF_SZ)) {
		size_t len;
		char *line = Rt_SkipWhitespace(buff), *ptr;
		uint32_t a, b, c, d, map;

		if (*line == 0x0 || *line == ';')
			continue;

		len = strlen(line);

		if (!strncmp(line, "[VirtualAxis]", len)) {
			mode = CFG_VA;
		} else if (!strncmp(line, "[AxisMapping]", len)) {
			mode = CFG_AM;
		} else if (!strncmp(line, "[KeyMapping]", len)) {
			mode = CFG_KM;
		} else {
			switch (mode) {
			case CFG_VA: {
				ptr = strchr(line, '=');
				*ptr++ = 0x0;
				a = atoi(ptr);

				ptr = strchr(ptr, ';') + 1;
				b = atoi(ptr);

				mbstowcs(wbuff, buff, BUFF_SZ);
				In_CreateVirtualAxis(wbuff, (enum Button)a, (enum Button)b);
			} break;
			case CFG_AM: {
				ptr = strchr(line, '=');
				*ptr++ = 0x0;
				a = atoi(ptr);

				ptr = strchr(ptr, ',') + 1;
				b = atoi(ptr);

				ptr = strchr(ptr, ';') + 1;
				c = atoi(ptr);

				ptr = strchr(ptr, ',') + 1;
				d = atoi(ptr);

				mbstowcs(wbuff, buff, BUFF_SZ);
				map = In_CreateMap(wbuff);

				In_MapPrimaryAxis(map, (enum Axis)a, (uint8_t)b);
				In_MapSecondaryAxis(map, (enum Axis)c, (uint8_t)d);
			} break;
			case CFG_KM: {
				ptr = strchr(line, '=');
				*ptr++ = 0x0;
				a = atoi(ptr);

				ptr = strchr(ptr, ',') + 1;
				b = atoi(ptr);

				ptr = strchr(ptr, ';') + 1;
				c = atoi(ptr);

				ptr = strchr(ptr, ',') + 1;
				d = atoi(ptr);

				mbstowcs(wbuff, buff, BUFF_SZ);
				map = In_CreateMap(wbuff);

				In_MapPrimaryButton(map, (enum Button)a, (uint8_t)b);
				In_MapSecondaryButton(map, (enum Button)c, (uint8_t)d);
			} break;
			}
		}

		memset(buff, 0x0, BUFF_SZ);
	}

	E_CloseStream(&stm);

	return true;
}

void
In_TermInput(void)
{
	Rt_TermArray(&_map);
	Rt_TermArray(&_virtualAxis);

	if (In_pointerCaptured)
		In_CapturePointer(false);

	if (!In_pointerVisible)
		In_ShowPointer(true);

	In_SysTerm();
}

void
In_Update(void)
{
	memmove(_prevButtonState, In_buttonState, sizeof(_prevButtonState));
	memmove(_prevControllerState, In_controllerState, sizeof(_prevControllerState));

	In_SysPollControllers();
	
	if (!_enableMouseAxis || !In_pointerCaptured)
		return;
	
	uint16_t x = 0, y = 0, hwidth = *E_screenWidth / 2, hheight = *E_screenHeight / 2;
	float dx = 0.f, dy = 0.f;
	
	In_PointerPosition(&x, &y);
	dx = (float)(hwidth - x);
	dy = (float)(hheight - y);
	In_SetPointerPosition(hwidth, hheight);
	
	In_mouseAxis[AXIS_MOUSE_X - MOUSE_AXIS_START] = -(dx / hwidth);
	In_mouseAxis[AXIS_MOUSE_Y - MOUSE_AXIS_START] = dy / hheight;
	
	// TODO: wheel support
}

void
In_EnableMouseAxis(bool enable)
{
	_enableMouseAxis = enable;
}

enum Axis
In_CreateVirtualAxis(const wchar_t *name, enum Button min, enum Button max)
{
	uint32_t axis = 0;
	const struct KeyAxis ka = { max, min, Rt_HashStringW(name) };
	
	if (!_virtualAxis.count)
		Rt_InitArray(&_virtualAxis, sizeof(struct KeyAxis), 10);
	
	axis = (enum Axis)(_virtualAxis.count + VIRTUAL_AXIS_START);
	Rt_ArrayAdd(&_virtualAxis, &ka);

	return (enum Axis)axis;
}

enum Axis
In_GetVirtualAxis(const wchar_t *name)
{
	size_t i;
	struct KeyAxis *ka;
	uint64_t hash = Rt_HashStringW(name);

	for (i = 0; i < _virtualAxis.count; ++i) {
		ka = Rt_ArrayGet(&_virtualAxis, i);
		if (ka->hash == hash)
			return (enum Axis)(i + VIRTUAL_AXIS_START);
	}

	return AXIS_UNRECOGNIZED;
}

uint32_t
In_CreateMap(const wchar_t *name)
{
	size_t i;
	uint32_t id = 0;
	struct Map m, *em;
	m.hash = Rt_HashStringW(name);

	if (!_map.size)
		Rt_InitArray(&_map, 10, sizeof(struct Map));

	for (i = 0; i < _map.count; ++i) {
		em = Rt_ArrayGet(&_map, i);
		if (em->hash == m.hash)
			return (uint32_t)i;
	}

	id = (uint32_t)_map.count;
	Rt_ArrayAdd(&_map, &m);

	return id;
}

void
In_MapPrimaryButton(uint32_t map, enum Button btn, uint8_t controller)
{
	struct Map *m = Rt_ArrayGet(&_map, map);
	m->key.primary = btn;
	m->key.primaryDevice = controller;
}

void
In_MapSecondaryButton(uint32_t map, enum Button btn, uint8_t controller)
{
	struct Map *m = Rt_ArrayGet(&_map, map);
	m->key.secondary = btn;
	m->key.secondaryDevice = controller;
}

void
In_MapPrimaryAxis(uint32_t map, enum Axis axis, uint8_t controller)
{
	struct Map *m = Rt_ArrayGet(&_map, map);
	m->axis.primary = axis;
	m->axis.primaryDevice = controller;
}

void
In_MapSecondaryAxis(uint32_t map, enum Axis axis, uint8_t controller)
{
	struct Map *m = Rt_ArrayGet(&_map, map);
	m->axis.secondary = axis;
	m->axis.secondaryDevice = controller;
}

bool
In_Button(uint32_t map)
{
	struct Map *m = Rt_ArrayGet(&_map, map);
	return In_UnmappedButton(m->key.primary, m->key.primaryDevice) || In_UnmappedButton(m->key.secondary, m->key.secondaryDevice);
}

bool
In_ButtonUp(uint32_t map)
{
	struct Map *m = Rt_ArrayGet(&_map, map);
	return In_UnmappedButtonUp(m->key.primary, m->key.primaryDevice) || In_UnmappedButtonUp(m->key.secondary, m->key.secondaryDevice);
}

bool
In_ButtonDown(uint32_t map)
{
	struct Map *m = Rt_ArrayGet(&_map, map);
	return In_UnmappedButtonDown(m->key.primary, m->key.primaryDevice) || In_UnmappedButtonDown(m->key.secondary, m->key.secondaryDevice);
}

float
In_Axis(uint32_t map)
{
	struct Map *m = Rt_ArrayGet(&_map, map);
	return In_UnmappedAxis(m->axis.primary, m->axis.primaryDevice) + In_UnmappedAxis(m->axis.secondary, m->axis.secondaryDevice);
}

bool
In_UnmappedButton(enum Button btn, uint8_t controller)
{
	return btn < BTN_STATE_COUNT ? In_buttonState[btn] : GPAD_BTN(btn, In_controllerState[controller]);
}

bool
In_UnmappedButtonUp(enum Button btn, uint8_t controller)
{
	const bool prev = btn < BTN_STATE_COUNT ? _prevButtonState[btn] : GPAD_BTN(btn, _prevControllerState[controller]);
	const bool cur = btn < BTN_STATE_COUNT ? In_buttonState[btn] : GPAD_BTN(btn, In_controllerState[controller]);
	return prev && !cur;
}

bool
In_UnmappedButtonDown(enum Button btn, uint8_t controller)
{
	const bool prev = btn < BTN_STATE_COUNT ? _prevButtonState[btn] : GPAD_BTN(btn, _prevControllerState[controller]);
	const bool cur = btn < BTN_STATE_COUNT ? In_buttonState[btn] : GPAD_BTN(btn, In_controllerState[controller]);
	return !prev && cur;
}

float
In_UnmappedAxis(enum Axis axis, uint8_t controller)
{
	struct KeyAxis *ka;
	float ret;

	if (axis < CONTROLLER_AXIS_COUNT) {
		return In_controllerState[controller].axis[axis];
	} else if (axis == AXIS_MOUSE_X || axis == AXIS_MOUSE_Y) {
		return 0.f;
	} else {
		ka = Rt_ArrayGet(&_virtualAxis, (size_t)axis - VIRTUAL_AXIS_START);
		if (!ka)
			return 0.f;

		ret = 0.f;
		if (In_UnmappedButton(ka->max, controller))
			ret += 1.f;
		if (In_UnmappedButton(ka->min, controller))
			ret -= 1.f;

		return ret;
	}
}

