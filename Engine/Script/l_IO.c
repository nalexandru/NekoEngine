#include <Engine/IO.h>
#include <Engine/Config.h>
#include <Script/Interface.h>
#include <System/Log.h>

#define SIO_MOD	"ScriptIO"

static int32_t *_readBufferSize = NULL;

SIF_FUNC(FileExists)
{
	lua_pushboolean(vm, E_FileExists(luaL_checkstring(vm, 1)));
	return 1;
}

SIF_FUNC(ListFiles)
{
	const char **files = E_ListFiles(luaL_checkstring(vm, 1));

	uint32_t count = 0;
	for (const char **i = files; *i; ++i)
		++count;

	uint32_t idx = 1;
	lua_createtable(vm, count, 0);
	for (const char **i = files; *i; ++i) {
		lua_pushstring(vm, *i);
		lua_rawseti(vm, -2, idx++);
	}

	E_FreeFileList(files);

	return 1;
}

SIF_FUNC(IsDirectory)
{
	lua_pushboolean(vm, E_IsDirectory(luaL_checkstring(vm, 1)));
	return 1;
}

SIF_FUNC(ProcessFiles)
{
	luaL_error(vm, "not implemented");
	return 0;
}

SIF_FUNC(EnableWrite)
{
	lua_pushboolean(vm, E_EnableWrite((enum NeWriteDirectory)luaL_checkinteger(vm, 1)));
	return 1;
}

SIF_FUNC(DisableWrite)
{
	E_DisableWrite();
	return 0;
}

SIF_FUNC(CreateDirectory)
{
	lua_pushboolean(vm, E_CreateDirectory(luaL_checkstring(vm, 1)));
	return 1;
}

SIF_FUNC(FileStream)
{
	const char *path = luaL_checkstring(vm, 1);
	int mode = (int)luaL_checkinteger(vm, 2);

	struct NeStream stm;
	if (!E_FileStream(path, mode, &stm)) {
		lua_pushnil(vm);
		return 1;
	}

	struct NeStream *ud = lua_newuserdatauv(vm, sizeof(*ud), 0);
	memcpy(ud, &stm, sizeof(*ud));
	return 1;
}

SIF_FUNC(MemoryStream)
{
	luaL_error(vm, "not implemented");
	return 0;
}

SIF_FUNC(Tell)
{
	lua_pushinteger(vm, E_StreamTell(luaL_checkudata(vm, 1, SIF_NE_STREAM)));
	return 1;
}

SIF_FUNC(Seek)
{
	lua_pushinteger(vm, E_SeekStream(luaL_checkudata(vm, 1, SIF_NE_STREAM), luaL_checkinteger(vm, 2),
										(enum NeFileSeekStart)luaL_checkinteger(vm, 3)));
	return 1;
}

SIF_FUNC(Length)
{
	lua_pushinteger(vm, E_StreamLength(luaL_checkudata(vm, 1, SIF_NE_STREAM)));
	return 1;
}

SIF_FUNC(End)
{
	lua_pushboolean(vm, E_EndOfStream(luaL_checkudata(vm, 1, SIF_NE_STREAM)));
	return 1;
}

SIF_FUNC(Read)
{
	int64_t ch = luaL_checkinteger(vm, 2);
	if (ch <= 0)
		luaL_argerror(vm, 2, "Number of characters must be positive.");

	char *dst = Sys_Alloc(sizeof(*dst), ch, MH_Frame);
	if (!dst)
		lua_pushnil(vm);

	E_ReadStreamLine(luaL_checkudata(vm, 1, SIF_NE_STREAM), dst, ch);
	lua_pushstring(vm, dst);
	return 1;
}

SIF_FUNC(ReadLine)
{
	char *dst = Sys_Alloc(sizeof(*dst), *_readBufferSize, MH_Frame);
	if (!dst)
		lua_pushnil(vm);

	E_ReadStreamLine(luaL_checkudata(vm, 1, SIF_NE_STREAM), dst, *_readBufferSize);
	lua_pushstring(vm, dst);
	return 1;
}

SIF_FUNC(Write)
{
	const char *str = luaL_checkstring(vm, 2);
	E_WriteStream(luaL_checkudata(vm, 1, SIF_NE_STREAM), str, strlen(str));
	return 0;
}

SIF_FUNC(Close)
{
	E_CloseStream(luaL_checkudata(vm, 1, SIF_NE_STREAM));
	return 0;
}

SIF_FUNC(__gc)
{
	struct NeStream *stm = luaL_testudata(vm, 1, SIF_NE_STREAM);
	if (stm && stm->type != ST_Closed)
		Sys_LogEntry(SIO_MOD, LOG_WARNING, "Stream not closed by script");
	return 0;
}

SIF_FUNC(__tostring)
{
	if (luaL_testudata(vm, 1, SIF_NE_STREAM)) {
		lua_pushliteral(vm, "NeStream");
		return 1;
	}

	return 0;
}

NE_SCRIPT_INTEFACE(NeIO)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_REG(__gc),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(Read),
		SIF_REG(ReadLine),
		SIF_REG(Write),
		SIF_REG(Tell),
		SIF_REG(Seek),
		SIF_REG(Length),
		SIF_REG(End),
		SIF_REG(Close),
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, SIF_NE_STREAM);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	luaL_Reg reg[] =
	{
		SIF_REG(FileStream),
		SIF_REG(MemoryStream),
		SIF_REG(FileExists),
		SIF_REG(ListFiles),
		SIF_REG(IsDirectory),
		SIF_REG(ProcessFiles),
		SIF_REG(EnableWrite),
		SIF_REG(DisableWrite),
		SIF_REG(CreateDirectory),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "IO");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, IO_READ);
		lua_setfield(vm, -2, "Read");
		lua_pushinteger(vm, IO_WRITE);
		lua_setfield(vm, -2, "Write");
		lua_pushinteger(vm, IO_APPEND);
		lua_setfield(vm, -2, "Append");
	}
	lua_setglobal(vm, "OpenMode");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, IO_SEEK_SET);
		lua_setfield(vm, -2, "Set");
		lua_pushinteger(vm, IO_SEEK_CUR);
		lua_setfield(vm, -2, "Cur");
		lua_pushinteger(vm, IO_SEEK_END);
		lua_setfield(vm, -2, "End");
	}
	lua_setglobal(vm, "Seek");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, WD_Data);
		lua_setfield(vm, -2, "Data");
		lua_pushinteger(vm, WD_Save);
		lua_setfield(vm, -2, "Save");
		lua_pushinteger(vm, WD_Temp);
		lua_setfield(vm, -2, "Temp");
		lua_pushinteger(vm, WD_Config);
		lua_setfield(vm, -2, "Config");
	}
	lua_setglobal(vm, "WriteDirectory");

	if (!_readBufferSize)
		_readBufferSize = &E_GetCVarI32("Script_ReadBufferSize", 1024)->i32;

	return 1;
}

/* NekoEngine
 *
 * l_IO.c
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
