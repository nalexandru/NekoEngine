#include <Input/Input.h>
#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <Engine/Console.h>
#include <Runtime/Runtime.h>
#include <System/Memory.h>
#include <System/Log.h>

#define BUFF_SZ		64

#define CFG_INIT	0
#define CFG_VA		1
#define CFG_AM		2
#define CFG_KM		3

struct NeKeyAxis
{
	enum NeButton max;
	enum NeButton min;
	uint64_t hash;
};

struct NeInputMap
{
	union {
		struct {
			enum NeButton primary;
			uint8_t primaryDevice;
			enum NeButton secondary;
			uint8_t secondaryDevice;
		} key;
		struct {
			enum NeAxis primary;
			uint8_t primaryDevice;
			enum NeAxis secondary;
			uint8_t secondaryDevice;
		} axis;
	};
	uint64_t hash;
};

bool In_pointerVisible = true;
bool In_pointerCaptured = false;
bool In_buttonState[BTN_STATE_COUNT];
float In_mouseAxis[3] = { 0.f, 0.f, 0.f };
float In_axisSensivity[255];
uint8_t In_connectedControllers = 0;
struct NeControllerState In_controllerState[IN_MAX_CONTROLLERS];

bool __InSys_enableMouseAxis = false;
extern bool __InSys_rawMouseAxis;

static bool _prevButtonState[BTN_STATE_COUNT];
static struct NeControllerState _prevControllerState[IN_MAX_CONTROLLERS];
static struct NeArray _map;
static struct NeArray _virtualAxis;

#define GPAD_BTN(x, state) (state.buttons & (1 << (x - BTN_GPAD_BTN_BASE)))

bool
In_InitInput(void)
{
	char *buff = NULL;
	struct NeStream stm;
	uint8_t mode = CFG_INIT;

	memset(In_buttonState, 0x0, sizeof(In_buttonState));
	memset(In_controllerState, 0x0, sizeof(In_controllerState));

	for (uint32_t i = 0; i < sizeof(In_axisSensivity) / sizeof(In_axisSensivity[0]); ++i)
		In_axisSensivity[i] = 1.f;

	In_axisSensivity[AXIS_MOUSE_X] = 10.f;
	In_axisSensivity[AXIS_MOUSE_Y] = 10.f;

	if (!In_SysInit())
		return false;

	if (!E_FileStream("/Config/Input.ini", IO_READ, &stm))
		return true;

	buff = Sys_Alloc(sizeof(char), BUFF_SZ, MH_Transient);

	while (E_ReadStreamLine(&stm, buff, BUFF_SZ)) {
		size_t len;
		char *line = Rt_SkipWhitespace(buff), *ptr;
		uint32_t a, b, c, d, map;

		if (*line == 0x0 || *line == ';')
			continue;

		len = strnlen(line, BUFF_SZ);

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

				In_CreateVirtualAxis(buff, (enum NeButton)a, (enum NeButton)b);
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

				map = In_CreateMap(buff);

				In_MapPrimaryAxis(map, (enum NeAxis)a, (uint8_t)b);
				In_MapSecondaryAxis(map, (enum NeAxis)c, (uint8_t)d);
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

				map = In_CreateMap(buff);

				In_MapPrimaryButton(map, (enum NeButton)a, (uint8_t)b);
				In_MapSecondaryButton(map, (enum NeButton)c, (uint8_t)d);
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
In_Key(enum NeButton key, bool down)
{
	if (E_ConsoleKey(key, down))
		return;

	In_buttonState[key] = down;
}

void
In_Update(void)
{
	memmove(_prevButtonState, In_buttonState, sizeof(_prevButtonState));
	memmove(_prevControllerState, In_controllerState, sizeof(_prevControllerState));

	In_SysPollControllers();

	if (!__InSys_enableMouseAxis || !In_pointerCaptured)
		return;

	if (__InSys_rawMouseAxis) {
		In_mouseAxis[0] = In_mouseAxis[1] = 0.f;
	} else {
		uint16_t x = 0, y = 0, hwidth = *E_screenWidth / 2, hheight = *E_screenHeight / 2;
		float dx = 0.f, dy = 0.f;
	
		In_PointerPosition(&x, &y);
		dx = (float)(hwidth - x);
		dy = (float)(hheight - y);
		In_SetPointerPosition(hwidth, hheight);
	
		In_mouseAxis[AXIS_MOUSE_X - MOUSE_AXIS_START] = dx / hwidth;
		In_mouseAxis[AXIS_MOUSE_Y - MOUSE_AXIS_START] = dy / hheight;

		// TODO: wheel support
	}
}

void
In_EnableMouseAxis(bool enable)
{
	__InSys_enableMouseAxis = enable;

	if (enable) {
		if (!In_pointerCaptured) {
			In_CapturePointer(true);
			In_ShowPointer(false);
		}
	} else {
		if (In_pointerCaptured) {
			In_CapturePointer(false);
			In_ShowPointer(true);
		}
		
		In_SetPointerPosition(*E_screenWidth / 2, *E_screenHeight / 2);
	}
}

enum NeAxis
In_CreateVirtualAxis(const char *name, enum NeButton min, enum NeButton max)
{
	uint32_t axis = 0;
	const struct NeKeyAxis ka = { max, min, Rt_HashString(name) };

	if (!_virtualAxis.count)
		Rt_InitArray(&_virtualAxis, sizeof(struct NeKeyAxis), 10, MH_System);

	axis = (enum NeAxis)(_virtualAxis.count + VIRTUAL_AXIS_START);
	Rt_ArrayAdd(&_virtualAxis, &ka);

	return (enum NeAxis)axis;
}

enum NeAxis
In_GetVirtualAxis(const char *name)
{
	size_t i;
	struct NeKeyAxis *ka;
	uint64_t hash = Rt_HashString(name);

	for (i = 0; i < _virtualAxis.count; ++i) {
		ka = Rt_ArrayGet(&_virtualAxis, i);
		if (ka->hash == hash)
			return (enum NeAxis)(i + VIRTUAL_AXIS_START);
	}

	return AXIS_UNRECOGNIZED;
}

uint32_t
In_CreateMap(const char *name)
{
	size_t i;
	uint32_t id = 0;
	struct NeInputMap m, *em;
	m.hash = Rt_HashString(name);

	if (!_map.size)
		Rt_InitArray(&_map, 10, sizeof(struct NeInputMap), MH_System);

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
In_MapPrimaryButton(uint32_t map, enum NeButton btn, uint8_t controller)
{
	struct NeInputMap *m = Rt_ArrayGet(&_map, map);
	m->key.primary = btn;
	m->key.primaryDevice = controller;
}

void
In_MapSecondaryButton(uint32_t map, enum NeButton btn, uint8_t controller)
{
	struct NeInputMap *m = Rt_ArrayGet(&_map, map);
	m->key.secondary = btn;
	m->key.secondaryDevice = controller;
}

void
In_MapPrimaryAxis(uint32_t map, enum NeAxis axis, uint8_t controller)
{
	struct NeInputMap *m = Rt_ArrayGet(&_map, map);
	m->axis.primary = axis;
	m->axis.primaryDevice = controller;
}

void
In_MapSecondaryAxis(uint32_t map, enum NeAxis axis, uint8_t controller)
{
	struct NeInputMap *m = Rt_ArrayGet(&_map, map);
	m->axis.secondary = axis;
	m->axis.secondaryDevice = controller;
}

bool
In_Button(uint32_t map)
{
	struct NeInputMap *m = Rt_ArrayGet(&_map, map);
	return In_UnmappedButton(m->key.primary, m->key.primaryDevice) || In_UnmappedButton(m->key.secondary, m->key.secondaryDevice);
}

bool
In_ButtonUp(uint32_t map)
{
	struct NeInputMap *m = Rt_ArrayGet(&_map, map);
	return In_UnmappedButtonUp(m->key.primary, m->key.primaryDevice) || In_UnmappedButtonUp(m->key.secondary, m->key.secondaryDevice);
}

bool
In_ButtonDown(uint32_t map)
{
	struct NeInputMap *m = Rt_ArrayGet(&_map, map);
	return In_UnmappedButtonDown(m->key.primary, m->key.primaryDevice) || In_UnmappedButtonDown(m->key.secondary, m->key.secondaryDevice);
}

float
In_Axis(uint32_t map)
{
	struct NeInputMap *m = Rt_ArrayGet(&_map, map);
	return In_UnmappedAxis(m->axis.primary, m->axis.primaryDevice) + In_UnmappedAxis(m->axis.secondary, m->axis.secondaryDevice);
}

bool
In_UnmappedButton(enum NeButton btn, uint8_t controller)
{
	return btn < BTN_STATE_COUNT ? In_buttonState[btn] : GPAD_BTN(btn, In_controllerState[controller]);
}

bool
In_UnmappedButtonUp(enum NeButton btn, uint8_t controller)
{
	const bool prev = btn < BTN_STATE_COUNT ? _prevButtonState[btn] : GPAD_BTN(btn, _prevControllerState[controller]);
	const bool cur = btn < BTN_STATE_COUNT ? In_buttonState[btn] : GPAD_BTN(btn, In_controllerState[controller]);
	return prev && !cur;
}

bool
In_UnmappedButtonDown(enum NeButton btn, uint8_t controller)
{
	const bool prev = btn < BTN_STATE_COUNT ? _prevButtonState[btn] : GPAD_BTN(btn, _prevControllerState[controller]);
	const bool cur = btn < BTN_STATE_COUNT ? In_buttonState[btn] : GPAD_BTN(btn, In_controllerState[controller]);
	return !prev && cur;
}

float
In_UnmappedAxis(enum NeAxis axis, uint8_t controller)
{
	struct NeKeyAxis *ka;
	float ret;

	if (axis < CONTROLLER_AXIS_COUNT) {
		return In_controllerState[controller].axis[axis] * In_axisSensivity[axis];
	} else if (axis == AXIS_MOUSE_X || axis == AXIS_MOUSE_Y) {
		return In_mouseAxis[axis - MOUSE_AXIS_START] * In_axisSensivity[axis];
	} else {
		ka = Rt_ArrayGet(&_virtualAxis, (size_t)axis - VIRTUAL_AXIS_START);
		if (!ka)
			return 0.f;

		ret = 0.f;
		if (In_UnmappedButton(ka->max, controller))
			ret += 1.f;
		if (In_UnmappedButton(ka->min, controller))
			ret -= 1.f;

		return ret * In_axisSensivity[axis];
	}
}

static char _keycodeMap[] =
{
	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
	'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',
	'k',  'l',  'm',  'n',  'o',  'p',  'q',  'r',  's',  't',
	'u',  'v',  'w',  'x',  'y',  'z',  0x0,  0x0,  0x0,  0x0,
	' ',  '`',  '\t', 0x0,  '\r', 0x0,  0x0,  0x0,  0x0,  0x0,
	0x0,  0x0,  0x0,  '/',  '\\', ',',  '.',  ';',  '\'', 0x0,
	'=',  '-',  0x0,  '[',  ']',  0x0,  0x0,  0x0,  0x0,  0x0,
	0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
	0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
	0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  '0',
	'1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '+',
	'-',  '.',  '/',  '*',  '\r', 0x0
};

char
In_KeycodeToChar(enum NeButton key, bool shift)
{
	char ch = _keycodeMap[key];

	if (shift) {
		if (ch > 0x60 && ch < 0x7B) {
			ch -= 0x20;
		} else if (ch > 0x5A && ch < 0x5E) {
			ch += 0x20;
		} else {
			switch (ch) {
			case 0x3D: return 0x2B;
			case 0x2C: return 0x3C;
			case 0x2D: return 0x5F;
			case 0x2E: return 0x3E;
			case 0x2F: return 0x3F;
			case 0x30: return 0x29;
			case 0x31: return 0x21;
			case 0x32: return 0x40;
			case 0x33: return 0x23;
			case 0x34: return 0x24;
			case 0x35: return 0x25;
			case 0x36: return 0x5E;
			case 0x37: return 0x26;
			case 0x38: return 0x2A;
			case 0x39: return 0x28;
			case '\'': return '"';
			}
		}
	}

	return ch;
}
