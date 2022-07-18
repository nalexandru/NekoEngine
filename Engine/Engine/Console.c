#include <stdarg.h>

#include <UI/UI.h>
#include <Input/Input.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Script/Script.h>
#include <Runtime/Runtime.h>

static bool _open, _enabled;
static lua_State *_consoleVM;
static struct NeArray _text, _line, _history;
static uint32_t _lineLength = 81;
static size_t _historyId = 0;
static uint32_t *_screenBufferSize = NULL;
static struct NeUIContext *_consoleCtx = NULL;

static inline void
_AppendText(const char *text)
{
	if (*_screenBufferSize == _text.count)
		Rt_ArrayRemove(&_text, 0);

	Rt_ArrayAdd(&_text, text);
}

bool
E_InitConsole(void)
{
	_enabled = E_GetCVarBln("Console_Enable", true)->bln;

	if (!_enabled)
		return true;

	_consoleVM = Sc_CreateVM();
	if (!_consoleVM)
		return false;

	_lineLength = ((uint32_t)*E_screenWidth / 12) + 1;
	_screenBufferSize = &E_GetCVarU32("Console_ScreenBufferSize", 50)->u32;

	Rt_InitArray(&_text, *_screenBufferSize, sizeof(char) * _lineLength, MH_System);
	Rt_InitArray(&_line, _lineLength, sizeof(char), MH_System);
	Rt_InitArray(&_history, 10, _lineLength * sizeof(char), MH_System);

	_consoleCtx = UI_CreateStandaloneContext(4000, 6000, *_screenBufferSize + 2);

	return true;
}

void
E_ConsolePuts(const char *s)
{
	if (!_enabled)
		return;

	size_t len = strlen(s);

	char *buff = Sys_Alloc(sizeof(*buff), _lineLength, MH_Transient);
	if (len > _lineLength) {
		const char *e = s + len;
		while (e - s >= _lineLength) {
			snprintf(buff, _lineLength, "%s", s);
			_AppendText(buff);
			s += _lineLength - 1;
		}
	}

	snprintf(buff, _lineLength, "%s", s);
	_AppendText(buff);
}

void
E_ConsolePrint(const char *fmt, ...)
{
	if (!_enabled)
		return;

	va_list args;
	char *buff = Sys_Alloc(sizeof(*buff), 2048, MH_Transient);

	va_start(args, fmt);
	vsnprintf(buff, 2048, fmt, args);
	va_end(args);

	E_ConsolePuts(buff);
}

void
E_ClearConsole(void)
{
	if (!_enabled)
		return;

	Rt_ClearArray(&_text, false);
}

void
E_ConsoleExec(const char *line)
{
	if (!_enabled)
		return;

	if (!strncmp(line, "set ", 4)) {
		const char *name = line + 4;
		struct NeCVar *cv = E_GetCVar(name);
		char *val = strchr(name, ' ');
		*val++ = 0x0;
		if (cv) {
			switch (cv->type) {
			case CV_String: E_SetCVarStr(name, val); break;
			case CV_Int32: cv->i32 = atoi(val); break;
			case CV_UInt32: cv->u32 = (uint32_t)strtoul(val, NULL, 10); break;
			case CV_UInt64: cv->u64 = strtoull(val, NULL, 10); break;
			case CV_Float: cv->flt = strtof(val, NULL); break;
			case CV_Bool: cv->bln = !strncmp(val, "true", strlen(val)); break;
			}
		} else {
			E_ConsolePrint("ERROR: Variable %hs does not exist", name);
		}
	} else if (!strncmp(line, "get ", 4)) {
		const char *name = line + 4;
		const struct NeCVar *cv = E_GetCVar(name);
		if (cv) {
			switch (cv->type) {
			case CV_String: E_ConsolePrint("%s [string] = %hs", name, cv->str); break;
			case CV_Int32: E_ConsolePrint("%s [int32] = %d", name, cv->i32); break;
			case CV_UInt32: E_ConsolePrint("%s [uint32] = %u", name, cv->u32); break;
			case CV_UInt64: E_ConsolePrint("%s [uint64] = %llu", name, cv->u64); break;
			case CV_Float: E_ConsolePrint("%s [float] = %f", name, cv->flt); break;
			case CV_Bool: E_ConsolePrint("%s [bool] = %s", name, cv->bln ? "true" : "false"); break;
			}
		}
		else {
			E_ConsolePrint("ERROR: Variable %hs does not exist", name);
		}
	} else if (!strncmp(line, "getall", 6)) {
		const struct NeCVar *cv = E_RootCVar();
		while (cv) {
			switch (cv->type) {
			case CV_String: E_ConsolePrint("%s [string] = %hs", cv->name, cv->str); break;
			case CV_Int32: E_ConsolePrint("%s [int32] = %d", cv->name, cv->i32); break;
			case CV_UInt32: E_ConsolePrint("%s [uint32] = %u", cv->name, cv->u32); break;
			case CV_UInt64: E_ConsolePrint("%s [uint64] = %llu", cv->name, cv->u64); break;
			case CV_Float: E_ConsolePrint("%s [float] = %f", cv->name, cv->flt); break;
			case CV_Bool: E_ConsolePrint("%s [bool] = %s", cv->name, cv->bln ? "true" : "false"); break;
			}

			cv = cv->next;
		}
	} else if (!strncmp(line, "exec ", 5)) {
		const char *err = Sc_ExecuteFile(_consoleVM, line + 5);
		if (err)
			E_ConsolePrint("ERROR: %hs", err);
	} else {
		const char *err = Sc_Execute(_consoleVM, line);
		if (err)
			E_ConsolePrint("ERROR: %hs", err);
	}
}

void
E_DrawConsole(void)
{
	if (!_enabled || !_open)
		return;

	float y = *E_screenHeight - 30.f;

	UI_DrawText(_consoleCtx, "> ", 0, y, 20, NULL);
	UI_DrawText(_consoleCtx, (const char *)_line.data, 20, y, 20, NULL);

	y -= 20 * (_text.count + 1);

	char *text;
	Rt_ArrayForEach(text, &_text) {
		UI_DrawText(_consoleCtx, text, 0, y, 20, NULL);
		y += 20;
	}
}

bool
E_ConsoleKey(enum NeButton key, bool down)
{
	if (!_enabled)
		return false;

	char ch = 0x0;
	static bool _shift = false;

	if (!_open && key != BTN_KEY_TILDE)
		return false;

	if (down) {
		switch (key) {
		case BTN_KEY_UP:
			if (!_history.count)
				break;

			Rt_ClearArray(&_line, false);
			snprintf((char *)_line.data, Rt_ArrayByteSize(&_line), "%s", (const char *)Rt_ArrayGet(&_history, _historyId));
			_line.count = strlen((const char *)_line.data);

			if (_historyId) --_historyId;
		break;
		case BTN_KEY_DOWN:
			if (!_history.count)
				break;

			if (_historyId == _history.count - 1) {
				Rt_ClearArray(&_line, false);
				memset(_line.data, 0x0, Rt_ArrayByteSize(&_line));
				break;
			}

			if (_historyId < _history.count - 1) ++_historyId;

			Rt_ClearArray(&_line, false);
			snprintf((char *)_line.data, Rt_ArrayByteSize(&_line), "%s", (const char *)Rt_ArrayGet(&_history, _historyId));
			_line.count = strlen((const char *)_line.data);
		break;
		case BTN_KEY_LEFT: break;
		case BTN_KEY_RIGHT: break;
		case BTN_KEY_RETURN:
			Rt_ArrayAdd(&_line, &ch);
			E_ConsoleExec((const char *)_line.data);

			Rt_ArrayAdd(&_history, _line.data);
			_historyId = _history.count - 1;

			Rt_ClearArray(&_line, false);
			memset(_line.data, 0x0, Rt_ArrayByteSize(&_line));
		break;
		case BTN_KEY_BKSPACE:
			if (_line.count) {
				--_line.count;
				_line.data[_line.count] = 0x0;
			}
		break;
		case BTN_KEY_DELETE: break;
		case BTN_KEY_TILDE: _open = !_open; break;
		case BTN_KEY_RSHIFT:
		case BTN_KEY_LSHIFT: _shift = true; break;
		default:
			ch = In_KeycodeToChar(key, _shift);
			if (ch != 0x0)
				Rt_ArrayAdd(&_line, &ch);
		break;
		}
	} else {
		if (key == BTN_KEY_RSHIFT || key == BTN_KEY_LSHIFT)
			_shift = false;
	}

	return true;
}

void
E_TermConsole(void)
{
	if (!_enabled)
		return;

	UI_DestroyStandaloneContext(_consoleCtx);
	Rt_TermArray(&_history);
	Rt_TermArray(&_line);
	Rt_TermArray(&_text);
	Sc_DestroyVM(_consoleVM);
}
