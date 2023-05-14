#include <System/Log.h>
#include <System/Endian.h>
#include <System/System.h>
#include <Script/Interface.h>

SIF_INTEGER(Time, Sys_Time());
SIF_BOOL(ScreenVisible, Sys_ScreenVisible());

SIF_FUNC(MessageBox)
{
	Sys_MessageBox(luaL_checkstring(vm, 1), luaL_checkstring(vm, 2), (int)luaL_optinteger(vm, 3, MSG_ICON_NONE));
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
SIF_STRING(OS, Sys_OperatingSystem());
SIF_STRING(OSVersion, Sys_OperatingSystemVersionString());
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

SIF_FUNC(Rand)
{
	lua_pushinteger(vm, rand());
	return 1;
}

NE_SCRIPT_INTEFACE(NeSystem)
{
	luaL_Reg reg[] =
	{
		SIF_REG(Time),
		SIF_REG(Sleep),
		SIF_REG(MSleep),
		SIF_REG(USleep),
		SIF_REG(Rand),
		SIF_REG(ScreenVisible),
		SIF_REG(Hostname),
		SIF_REG(Machine),
		SIF_REG(CpuName),
		SIF_REG(CpuFreq),
		SIF_REG(CpuCount),
		SIF_REG(CpuThreadCount),
		SIF_REG(TotalMemory),
		SIF_REG(FreeMemory),
		SIF_REG(OS),
		SIF_REG(OSVersion),
		SIF_REG(MachineType),
		SIF_REG(Capabilities),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "System");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, MSG_ICON_NONE);
		lua_setfield(vm, -2, "NoIcon");

		lua_pushinteger(vm, MSG_ICON_INFO);
		lua_setfield(vm, -2, "Information");

		lua_pushinteger(vm, MSG_ICON_WARN);
		lua_setfield(vm, -2, "Warning");

		lua_pushinteger(vm, MSG_ICON_ERROR);
		lua_setfield(vm, -2, "Error");

		lua_pushcfunction(vm, Sif_MessageBox);
		lua_setfield(vm, -2, "Show");
	}
	lua_setglobal(vm, "MessageBox");

	return 1;
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
