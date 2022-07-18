#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <physfs.h>

#include <Engine/IO.h>
#include <Engine/Config.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <System/System.h>
#include <Runtime/Runtime.h>
#include <Engine/Application.h>

#ifndef USE_PLATFORM_RESOURCES
#	include "EngineRes.h"
#	include "Shaders.h"
#else
bool Sys_MountPlatformResources(void);
void Sys_UnmountPlatformResources(void);
#endif

#define IO_MODULE "I/O"

const char *E_RealPath(const char *path);

NeFile
E_OpenFile(const char *path, enum NeFileOpenMode mode)
{
	PHYSFS_file *f = NULL;

	switch (mode) {
	case IO_READ:
		f = PHYSFS_openRead(path);
		break;
	case IO_WRITE:
		f = PHYSFS_openWrite(path);
		break;
	case IO_APPEND:
		f = PHYSFS_openAppend(path);
		break;
	}

	return (NeFile)f;
}

void *
E_MapFile(const char *path, enum NeFileOpenMode mode, uint64_t *size)
{
	void *ptr = NULL;
	if (!Sys_MapFile(E_RealPath(path), mode == IO_WRITE, &ptr, size))
		return NULL;

	return ptr;
}

void
E_UnmapFile(const void *ptr, uint64_t size)
{
	Sys_UnmapFile(ptr, size);
}

int64_t
E_ReadFile(NeFile f, void *ptr, int64_t size)
{
	return PHYSFS_readBytes((PHYSFS_file *)f, ptr, size);
}

void *
E_ReadFileBlob(NeFile f, int64_t *size, bool transient)
{
	uint8_t *ret = NULL;
	enum NeMemoryHeap heap = transient ? MH_Transient : MH_System;

	*size = PHYSFS_fileLength((PHYSFS_file *)f);
	if (!*size)
		return NULL;

	ret = Sys_Alloc(sizeof(char), (size_t)*size, heap);
	if (!ret)
		return NULL;

	if (PHYSFS_readBytes((PHYSFS_file *)f, ret, *size) != *size) {
		Sys_Free(ret);
		return NULL;
	}

	return ret;
}

char *
E_ReadFileText(NeFile f, int64_t *size, bool transient)
{
	char *ret = NULL;
	enum NeMemoryHeap heap = transient ? MH_Transient : MH_System;

	*size = PHYSFS_fileLength((PHYSFS_file *)f) - PHYSFS_tell((PHYSFS_file *)f);
	if (!*size)
		return NULL;

	ret = Sys_Alloc(sizeof(char), (size_t)*size + 1, heap);
	if (!ret)
		return NULL;

	if (PHYSFS_readBytes((PHYSFS_file *)f, ret, *size) != *size) {
		Sys_Free(ret);
		return NULL;
	}

	ret[*size - 1] = 0x0;

	return ret;
}

char *
E_FGets(NeFile f, char *buff, int64_t max)
{
	char c;
	char *p = buff;

	if (!f)
		return NULL;

	memset(buff, 0x0, (size_t)max);

	do {
		if (E_ReadFile(f, &c, 1) <= 0)
			return NULL;
	}
	while (c == ' ' || c == '\t');

	*p = c;
	++p;

	do {
		if (E_ReadFile(f, &c, 1) <= 0) {
			if (E_FEof(f)) {
				if (p == buff)
					return NULL;

				*p = 0x0;
				return buff;
			} else {
				return NULL;
			}
		}

		if (c == '\r') {
			*p = 0x0;
		} else if (c == '\n') {
			*p = 0x0;
			return buff;
		} else {
			*p = c;
			++p;
		}
	}
	while (1);
}

int64_t
E_WriteFile(NeFile f, const void *ptr, int64_t size)
{
	return PHYSFS_writeBytes((PHYSFS_file *)f, ptr, size);
}

int64_t
E_FTell(NeFile f)
{
	return PHYSFS_tell((PHYSFS_file *)f);
}

int64_t
E_FSeek(NeFile f, int64_t offset, enum NeFileSeekStart whence)
{
	int64_t dest = offset;

	if (whence == IO_SEEK_CUR)
		dest += PHYSFS_tell((PHYSFS_file *)f);
	else if (whence == IO_SEEK_END)
		dest = PHYSFS_fileLength((PHYSFS_file *)f) - offset;

	return PHYSFS_seek((PHYSFS_file *)f, dest);
}

int64_t
E_FileLength(NeFile f)
{
	return PHYSFS_fileLength((PHYSFS_file *)f);
}

bool
E_FEof(NeFile f)
{
	return PHYSFS_eof((PHYSFS_file *)f);
}

bool
E_FileExists(const char *path)
{
	return PHYSFS_exists(path);
}

void
E_CloseFile(NeFile file)
{
	PHYSFS_close((PHYSFS_file *)file);
}

bool
E_Mount(const char *path, const char *point)
{
	if (!PHYSFS_mount(path, point, 0)) {
		Sys_LogEntry(IO_MODULE, LOG_CRITICAL,
			"Failed to mount directory [%s] -> [%s]: %s", path, point,
			PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return false;
	}

	return true;
}

bool
E_MountMemory(const char *name, const void *ptr, uint64_t size, const char *point)
{
	if (!PHYSFS_mountMemory(ptr, size, NULL, name, point, 0)) {
		Sys_LogEntry(IO_MODULE, LOG_CRITICAL,
			"Failed to mount memory archive [%s] -> [%s]: %s", name, point,
			PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return false;
	}
	
	return true;
}

bool
E_Unmount(const char *name)
{
	return PHYSFS_unmount(name);
}

const char **
E_ListFiles(const char *dir)
{
	return (const char **)PHYSFS_enumerateFiles(dir);
}

bool
E_IsDirectory(const char *path)
{
	PHYSFS_Stat st;
	PHYSFS_stat(path, &st);
	return st.filetype == PHYSFS_FILETYPE_DIRECTORY;
}

void
E_FreeFileList(const char **list)
{
	PHYSFS_freeList((void *)list);
}

void
E_ProcessFiles(const char *path, const char *ext, bool recurse, void (*cb)(const char *))
{
	size_t len = strlen(path);
	char **files = PHYSFS_enumerateFiles(path), **f = NULL, *eptr = NULL;
	char file[4096];

	for (f = files; *f; ++f) {
		snprintf(file, 4096, "%s/%s", len == 1 ? "" : path, *f);

		if (E_IsDirectory(file)) {
			if (!recurse)
				continue;

			E_ProcessFiles(file, ext, true, cb);
		} else {
			eptr = strrchr(*f, '.');
			if (!eptr)
				continue;

			++eptr;
			if (strncmp(eptr, ext, strlen(eptr)))
				continue;

			cb(file);
		}
	}

	PHYSFS_freeList((void *)files);
}

bool
E_EnableWrite(enum NeWriteDirectory wd)
{
	size_t len = 4096;
	char *dir = Sys_Alloc(sizeof(*dir), len, MH_Transient);

	switch (wd) {
	case WD_Data: memcpy(dir, E_GetCVarStr("Engine_DataDir", NULL)->str, len); break;
	case WD_Save: Sys_DirectoryPath(SD_SAVE_GAME, dir, len); break;
	case WD_Temp: Sys_DirectoryPath(SD_TEMP, dir, len); break;
	case WD_Config: Sys_DirectoryPath(SD_APP_DATA, dir, len); break;
	}

	return PHYSFS_setWriteDir(dir);
}

void
E_DisableWrite(void)
{
	PHYSFS_setWriteDir(NULL);
}

bool
E_CreateDirectory(const char *path)
{
	return PHYSFS_mkdir(path);
}

bool
E_FileStream(const char *path, enum NeFileOpenMode mode, struct NeStream *stm)
{
	if (!path || !strlen(path))
		return false;

	memset(stm, 0x0, sizeof(*stm));

	if (Sys_Capabilities() & SC_MMIO) {
		stm->ptr = E_MapFile(path, mode, &stm->size);
		stm->pos = 0;
		stm->type = ST_MappedFile;
		stm->f = NULL;
	}

	if (!stm->ptr) {
		stm->f = E_OpenFile(path, mode);
		if (!stm->f)
			return false;

		stm->size = (uint64_t)E_FileLength(stm->f);
		stm->pos = 0;
		stm->type = ST_File;
	}

	stm->open = stm->ptr || stm->f;
	return stm->open;
}

bool
E_MemoryStream(void *buff, uint64_t size, struct NeStream *stm)
{
	stm->open = false;
	return false;
}

void
E_CloseStream(struct NeStream *stm)
{
	if (stm->type == ST_File)
		E_CloseFile(stm->f);
	else if (stm->type == ST_MappedFile)
		E_UnmapFile(stm->ptr, stm->size);

	memset(stm, 0x0, sizeof(*stm));
}

bool
E_InitIOSystem(void)
{
	if (!PHYSFS_init("NekoEngine")) {
		Sys_LogEntry(IO_MODULE, LOG_CRITICAL, "Failed to initialize I/O subsystem: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return false;
	}

#ifndef USE_PLATFORM_RESOURCES
	if (!PHYSFS_mountMemory(EngineRes_zip, sizeof(EngineRes_zip), 0, "EngineRes.zip", "/", 0) ||
		!PHYSFS_mountMemory(Shaders_zip, sizeof(Shaders_zip), 0, "Shaders.zip", "/", 0)) {
#else
	if (!Sys_MountPlatformResources()) {
#endif
		Sys_LogEntry(IO_MODULE, LOG_CRITICAL, "Failed to load builtin resources: %ls", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return false;
	}

	const char *dataDir = E_GetCVarStr("Engine_DataDir", "Data")->str;
	if (!Sys_DirectoryExists(dataDir))
		Sys_CreateDirectory(dataDir);

	if (!PHYSFS_mount(dataDir, "/", 0))
		return false;

	size_t len = 4096;
	char *dir = Sys_Alloc(sizeof(*dir), len, MH_Transient);
	
	Sys_DirectoryPath(SD_APP_DATA, dir, len);
	strncat(dir, "/GameData.zip", 14);

	if (Sys_FileExists(dir))
		PHYSFS_mount(dir, "/", 0);
	
	Sys_DirectoryPath(SD_TEMP, dir, len);
	if (!PHYSFS_mount(dir, "/Temp", 1)) {
		Sys_LogEntry(IO_MODULE, LOG_CRITICAL, "Failed to mount Temp directory: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return false;
	}

	Sys_DirectoryPath(SD_SAVE_GAME, dir, len);
	if (!Sys_DirectoryExists(dir))
		Sys_CreateDirectory(dir);

	if (!PHYSFS_mount(dir, "/Save", 1)) {
		Sys_LogEntry(IO_MODULE, LOG_CRITICAL, "Failed to mount Save directory: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return false;
	}

	Sys_DirectoryPath(SD_APP_DATA, dir, len);
	if (!Sys_DirectoryExists(dir))
		Sys_CreateDirectory(dir);
	
	if (!PHYSFS_mount(dir, "/Config", 1))
		return false;

	return true;
}

void
E_TermIOSystem(void)
{
#ifndef USE_PLATFORM_RESOURCES
	if (!PHYSFS_unmount("EngineRes.zip") || !PHYSFS_unmount("Shaders.zip"))
		Sys_LogEntry(IO_MODULE, LOG_DEBUG, "Failed to unmount EngineRes.zip");
#else
	Sys_UnmountPlatformResources();
#endif

	PHYSFS_deinit();
}

const char *
E_RealPath(const char *path)
{
	const char *realDir = PHYSFS_getRealDir(path);
	if (!realDir)
		return NULL;

	char *realPath = Sys_Alloc(sizeof(char), 4096, MH_Transient);
	if (!realPath)
		return NULL;

	snprintf(realPath, 4096, "%s/%s", realDir, path);
	return realPath;
}
