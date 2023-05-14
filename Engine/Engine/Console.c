#include <stdarg.h>

#include <UI/UI.h>
#include <Input/Input.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Script/Script.h>
#include <Runtime/Runtime.h>
#include <Interfaces/ConsoleOutput.h>
#include <Engine/Plugin.h>
#include <System/Log.h>

#define CONSOLE_MOD		"Console"

static bool f_visible, f_enabled;
static lua_State *f_consoleVM;
static struct NeArray f_text, f_line, f_history;
static uint32_t f_lineLength = 81, f_screenBufferSize;
static size_t f_historyId = 0;
static struct NeConsoleOutput *f_output;

static inline void
AppendText(const char *text)
{
	if (f_screenBufferSize == f_text.count)
		Rt_ArrayRemove(&f_text, 0);

	Rt_ArrayAdd(&f_text, text);
}

bool
E_InitConsole(void)
{
	f_enabled = E_GetCVarBln("Console_Enable", true)->bln;
	if (!f_enabled)
		return true;

	f_output = E_GetInterface(NEIF_CONSOLE_OUTPUT);
	if (!f_output) {
		Sys_LogEntry(CONSOLE_MOD, LOG_CRITICAL, "Console output interface not found. The console will not be available");
		return true;
	}

	f_consoleVM = Sc_CreateVM();
	if (!f_consoleVM)
		return false;

	f_lineLength = ((uint32_t)*E_screenWidth / 12) + 1;
	f_screenBufferSize = E_GetCVarU32("Console_ScreenBufferSize", 50)->u32;

	Rt_InitArray(&f_text, f_screenBufferSize, sizeof(char) * f_lineLength, MH_System);
	Rt_InitArray(&f_line, f_lineLength, sizeof(char), MH_System);
	Rt_InitArray(&f_history, 10, f_lineLength * sizeof(char), MH_System);

	return f_output->Init(f_screenBufferSize + 2);
}

void
E_ConsolePuts(const char *s)
{
	if (!f_enabled)
		return;

	size_t len = strlen(s);

	char *buff = Sys_Alloc(sizeof(*buff), f_lineLength, MH_Transient);
	if (len > f_lineLength) {
		const char *e = s + len;
		while (e - s >= f_lineLength) {
			strlcpy(buff, s, f_lineLength);
			AppendText(buff);
			s += f_lineLength - 1;
		}
	}

	strlcpy(buff, s, f_lineLength);
	AppendText(buff);
}

void
E_ConsolePrint(const char *fmt, ...)
{
	if (!f_enabled)
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
	if (!f_enabled)
		return;

	Rt_ClearArray(&f_text, false);
}

void
E_ConsoleExec(const char *line)
{
	if (!f_enabled)
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
		const char *err = Sc_ExecuteFile(f_consoleVM, line + 5);
		if (err)
			E_ConsolePrint("ERROR: %hs", err);
	} else {
		const char *err = Sc_Execute(f_consoleVM, line);
		if (err)
			E_ConsolePrint("ERROR: %hs", err);
	}
}

void
E_DrawConsole(void)
{
	if (!f_enabled || !f_visible)
		return;

	const uint32_t lh = f_output->LineHeight();
	uint32_t y = *E_screenHeight - lh - 10;

	f_output->Puts("> ", 0, y);
	f_output->Puts((const char *)f_line.data, lh, y);

	y -= lh * ((uint32_t)f_text.count + 1);

	char *text;
	Rt_ArrayForEach(text, &f_text) {
		f_output->Puts(text, 0, y);
		y += lh;
	}
}

void
E_ShowConsole(bool visible)
{
	f_visible = visible;
}

bool
E_ConsoleVisible(void)
{
	return f_visible;
}

bool
E_ConsoleKey(enum NeButton key, bool down)
{
	if (!f_enabled)
		return false;

	char ch = 0x0;
	static bool _shift = false;

	if (!f_visible && key != BTN_KEY_TILDE)
		return false;

	if (down) {
		switch (key) {
		case BTN_KEY_UP:
			if (!f_history.count)
				break;

			Rt_ClearArray(&f_line, false);
			strlcpy((char *)f_line.data, (const char *)Rt_ArrayGet(&f_history, f_historyId), Rt_ArrayByteSize(&f_line));
			f_line.count = strlen((const char *)f_line.data);

			if (f_historyId) --f_historyId;
		break;
		case BTN_KEY_DOWN:
			if (!f_history.count)
				break;

			if (f_historyId == f_history.count - 1) {
				Rt_ClearArray(&f_line, false);
				memset(f_line.data, 0x0, Rt_ArrayByteSize(&f_line));
				break;
			}

			if (f_historyId < f_history.count - 1) ++f_historyId;

			Rt_ClearArray(&f_line, false);
			strlcpy((char *)f_line.data, (const char *)Rt_ArrayGet(&f_history, f_historyId), Rt_ArrayByteSize(&f_line));
			f_line.count = strlen((const char *)f_line.data);
		break;
		case BTN_KEY_LEFT: break;
		case BTN_KEY_RIGHT: break;
		case BTN_KEY_RETURN:
			Rt_ArrayAdd(&f_line, &ch);
			E_ConsoleExec((const char *)f_line.data);

			Rt_ArrayAdd(&f_history, f_line.data);
				f_historyId = f_history.count - 1;

			Rt_ClearArray(&f_line, false);
			memset(f_line.data, 0x0, Rt_ArrayByteSize(&f_line));
		break;
		case BTN_KEY_BKSPACE:
			if (f_line.count) {
				--f_line.count;
				f_line.data[f_line.count] = 0x0;
			}
		break;
		case BTN_KEY_DELETE: break;
		case BTN_KEY_TILDE: f_visible = !f_visible; break;
		case BTN_KEY_RSHIFT:
		case BTN_KEY_LSHIFT: _shift = true; break;
		default:
			ch = In_KeycodeToChar(key, _shift);
			if (ch != 0x0)
				Rt_ArrayAdd(&f_line, &ch);
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
	if (!f_enabled)
		return;

	f_output->Term();
	Rt_TermArray(&f_history);
	Rt_TermArray(&f_line);
	Rt_TermArray(&f_text);
	Sc_DestroyVM(f_consoleVM);
}

/* NekoEngine
 *
 * Console.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
