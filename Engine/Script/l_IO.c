#include <Engine/IO.h>
#include <Engine/Config.h>
#include <Script/Script.h>
#include <Engine/Console.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

static int32_t *_readBufferSize = NULL;

SIF_FUNC(OpenFile)
{
	NeFile f = E_OpenFile(luaL_checkstring(vm, 1), (enum NeFileOpenMode)luaL_checkinteger(vm, 2));
	if (f)
		lua_pushlightuserdata(vm, f);
	else
		lua_pushnil(vm);
	return 1;
}

SIF_FUNC(ReadFile)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	char *dst = Sys_Alloc(sizeof(*dst), *_readBufferSize, MH_Frame);

	E_FGets(lua_touserdata(vm, 1), dst, *_readBufferSize);

	if (dst)
		lua_pushstring(vm, dst);
	else
		lua_pushnil(vm);

	return 1;
}

SIF_FUNC(ReadFullFile)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	int64_t size = 0;
	char *txt = E_ReadFileText(lua_touserdata(vm, 1), &size, true);

	if (txt)
		lua_pushstring(vm, txt);
	else
		lua_pushnil(vm);

	return 1;
}

SIF_FUNC(WriteFile)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	const char *str = luaL_checkstring(vm, 2);

	E_WriteFile(lua_touserdata(vm, 1), str, strlen(str));

	return 0;
}

SIF_FUNC(FTell)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushinteger(vm, E_FTell(lua_touserdata(vm, 1)));
	return 1;
}

SIF_FUNC(FSeek)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushinteger(vm, E_FSeek(lua_touserdata(vm, 1), luaL_checkinteger(vm, 2), (enum NeFileSeekStart)luaL_checkinteger(vm, 3)));
	return 1;
}

SIF_FUNC(FileLength)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushinteger(vm, E_FileLength(lua_touserdata(vm, 1)));
	return 1;
}

SIF_FUNC(FEof)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushboolean(vm, E_FEof(lua_touserdata(vm, 1)));
	return 1;
}

SIF_FUNC(FileExists)
{
	lua_pushboolean(vm, E_FileExists(luaL_checkstring(vm, 1)));
	return 1;
}

SIF_FUNC(CloseFile)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	E_CloseFile(lua_touserdata(vm, 1));
	return 0;
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

	struct NeStream *stm = Sys_Alloc(sizeof(*stm), 1, MH_Script);
	if (!stm) {
		lua_pushnil(vm);
		return 1;
	}

	if (!E_FileStream(path, mode, stm)) {
		Sys_Free(stm);
		lua_pushnil(vm);
		return 1;
	}

	lua_pushlightuserdata(vm, stm);
	return 1;
}

SIF_FUNC(MemoryStream)
{
	luaL_error(vm, "not implemented");
	return 0;
}

SIF_FUNC(CloseStream)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	struct NeStream *stm = lua_touserdata(vm, 1);

	E_CloseStream(stm);

	Sys_Free(stm);

	return 0;
}

SIF_FUNC(StreamTell)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushinteger(vm, E_StreamTell(lua_touserdata(vm, 1)));
	return 1;
}

SIF_FUNC(StreamSeek)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushinteger(vm, E_StreamSeek(lua_touserdata(vm, 1), luaL_checkinteger(vm, 2), (enum NeFileSeekStart)luaL_checkinteger(vm, 3)));
	return 1;
}

SIF_FUNC(StreamLength)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushinteger(vm, E_StreamLength(lua_touserdata(vm, 1)));
	return 1;
}

SIF_FUNC(EndOfStream)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushboolean(vm, E_EndOfStream(lua_touserdata(vm, 1)));
	return 1;
}

SIF_FUNC(ReadStream)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	char *dst = Sys_Alloc(sizeof(*dst), *_readBufferSize, MH_Frame);

	E_ReadStreamLine(lua_touserdata(vm, 1), dst, *_readBufferSize);

	if (dst)
		lua_pushstring(vm, dst);
	else
		lua_pushnil(vm);

	return 1;
}

SIF_FUNC(WriteStream)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	const char *str = luaL_checkstring(vm, 2);

	E_WriteStream(lua_touserdata(vm, 1), str, strlen(str));

	return 0;
}

void
SIface_OpenIO(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(OpenFile),
		SIF_REG(ReadFile),
		SIF_REG(ReadFullFile),
		SIF_REG(WriteFile),
		SIF_REG(FTell),
		SIF_REG(FSeek),
		SIF_REG(FileLength),
		SIF_REG(FEof),
		SIF_REG(FileExists),
		SIF_REG(CloseFile),
		SIF_REG(ListFiles),
		SIF_REG(IsDirectory),
		SIF_REG(ProcessFiles),
		SIF_REG(EnableWrite),
		SIF_REG(DisableWrite),
		SIF_REG(CreateDirectory),
		SIF_REG(FileStream),
		SIF_REG(MemoryStream),
		SIF_REG(CloseStream),
		SIF_REG(StreamTell),
		SIF_REG(StreamSeek),
		SIF_REG(StreamLength),
		SIF_REG(EndOfStream),
		SIF_REG(ReadStream),
		SIF_REG(WriteStream),

		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "IO");

	lua_pushinteger(vm, IO_READ);
	lua_setglobal(vm, "IO_READ");

	lua_pushinteger(vm, IO_WRITE);
	lua_setglobal(vm, "IO_WRITE");

	lua_pushinteger(vm, IO_APPEND);
	lua_setglobal(vm, "IO_APPEND");

	lua_pushinteger(vm, IO_SEEK_SET);
	lua_setglobal(vm, "IO_SEEK_SET");

	lua_pushinteger(vm, IO_SEEK_CUR);
	lua_setglobal(vm, "IO_SEEK_CUR");

	lua_pushinteger(vm, IO_SEEK_END);
	lua_setglobal(vm, "IO_SEEK_END");

	lua_pushinteger(vm, WD_Data);
	lua_setglobal(vm, "WD_Data");

	lua_pushinteger(vm, WD_Save);
	lua_setglobal(vm, "WD_Save");

	lua_pushinteger(vm, WD_Temp);
	lua_setglobal(vm, "WD_Temp");

	lua_pushinteger(vm, WD_Config);
	lua_setglobal(vm, "WD_Config");

	if (!_readBufferSize)
		_readBufferSize = &E_GetCVarI32("Script_ReadBufferSize", 16384)->i32;
}
