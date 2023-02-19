#include <System/Log.h>
#include <System/Endian.h>
#include <System/System.h>
#include <Script/Script.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

SIF_INTEGER(Time, Sys_Time());
SIF_BOOL(ScreenVisible, Sys_ScreenVisible());

SIF_FUNC(MessageBox)
{
	const char *title = luaL_checkstring(vm, 1);
	const char *message = luaL_checkstring(vm, 2);
	const int icon = (int)luaL_checkinteger(vm, 3);

	if (!title || !message)
		return 0;

	Sys_MessageBox(title, message, icon);
	
	return 0;
}

SIF_STRING(Hostname, Sys_Hostname());
SIF_STRING(Machine, Sys_Machine());
SIF_STRING(CpuName, Sys_CpuName());
SIF_INTEGER(CpuFreq, Sys_CpuFreq());
SIF_INTEGER(CpuCount, Sys_CpuCount());
SIF_INTEGER(CpuThreadCount, Sys_CpuThreadCount());
SIF_INTEGER(TotalMemory, Sys_TotalMemory());
SIF_INTEGER(FreeMemory, Sys_FreeMemory());
SIF_STRING(OperatingSystem, Sys_OperatingSystem());
SIF_STRING(OperatingSystemVersionString, Sys_OperatingSystemVersionString());
SIF_INTEGER(MachineType, Sys_MachineType());
SIF_INTEGER(Capabilities, Sys_Capabilities());

SIF_FUNC(Sleep)
{
	Sys_Sleep((uint32_t)luaL_checkinteger(vm, 1));
	return 0;
}

SIF_FUNC(MSleep)
{
	Sys_MSleep((uint32_t)luaL_checkinteger(vm, 1));
	return 0;
}

SIF_FUNC(USleep)
{
	Sys_USleep((uint32_t)luaL_checkinteger(vm, 1));
	return 0;
}

SIF_FUNC(LogEntry)
{
	const char *module = luaL_checkstring(vm, 1);
	const int severity = (int)luaL_checkinteger(vm, 2);
	const char *message = luaL_checkstring(vm, 3);

	if (!module || !message)
		return 0;

	Sys_LogEntry(module, severity, message);

	return 0;
}

SIF_BOOL(BigEndian, Sys_BigEndian());

SIF_FUNC(Rand)
{
	lua_pushnumber(vm, rand());
	return 1;
}

/*void *Sys_LoadLibrary(const char *path);
void *Sys_GetProcAddress(void *lib, const char *name);
void Sys_UnloadLibrary(void *lib);*/

void
SIface_OpenSystem(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(Time),
		SIF_REG(Sleep),
		SIF_REG(MSleep),
		SIF_REG(USleep),
		SIF_REG(LogEntry),
		SIF_REG(Rand),
		SIF_REG(ScreenVisible),
		SIF_REG(MessageBox),
		SIF_REG(Hostname),
		SIF_REG(Machine),
		SIF_REG(CpuName),
		SIF_REG(CpuFreq),
		SIF_REG(CpuCount),
		SIF_REG(CpuThreadCount),
		SIF_REG(TotalMemory),
		SIF_REG(FreeMemory),
		SIF_REG(OperatingSystem),
		SIF_REG(OperatingSystemVersionString),
		SIF_REG(MachineType),
		SIF_REG(Capabilities),
		SIF_REG(BigEndian),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Sys");

	lua_pushinteger(vm, LOG_DEBUG);
	lua_setglobal(vm, "LOG_DEBUG");

	lua_pushinteger(vm, LOG_CRITICAL);
	lua_setglobal(vm, "LOG_CRITICAL");

	lua_pushinteger(vm, LOG_WARNING);
	lua_setglobal(vm, "LOG_WARNING");

	lua_pushinteger(vm, LOG_INFORMATION);
	lua_setglobal(vm, "LOG_INFORMATION");

	lua_pushinteger(vm, MSG_ICON_NONE);
	lua_setglobal(vm, "MSG_ICON_NONE");

	lua_pushinteger(vm, MSG_ICON_INFO);
	lua_setglobal(vm, "MSG_ICON_INFO");

	lua_pushinteger(vm, MSG_ICON_WARN);
	lua_setglobal(vm, "MSG_ICON_WARN");

	lua_pushinteger(vm, MSG_ICON_ERROR);
	lua_setglobal(vm, "MSG_ICON_ERROR");
}

/* NekoEngine
 *
 * l_System.c
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
