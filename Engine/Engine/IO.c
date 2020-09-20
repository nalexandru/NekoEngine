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

#define IO_MODULE L"I/O"

extern unsigned char engine_res[];
extern unsigned int engine_res_size;

File
E_OpenFile(const char *path, FileOpenMode mode)
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

	if (!f)
		Sys_LogEntry(IO_MODULE, LOG_CRITICAL,
			L"Failed to open file [%S], %S", path,
			PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

	return (File)f;
}

void *
E_MapFile(const char *path, FileOpenMode mode, uint64_t *size)
{
	void *ptr = NULL;
	const char *realDir = NULL;
	char *realPath = NULL;
	
	realDir = PHYSFS_getRealDir(path);
	if (!realDir)
		return NULL;

	realPath = Sys_Alloc(sizeof(char), 4096, MH_Transient);
	if (!realPath)
		return NULL;

	snprintf(realPath, 4096, "%s/%s", realDir, path);

	if (!Sys_MapFile(realPath, mode == IO_WRITE, &ptr, size))
		return NULL;

	return ptr;
}

void
E_UnmapFile(const void *ptr, uint64_t size)
{
	Sys_UnmapFile(ptr, size);
}

int64_t
E_ReadFile(File f, void *ptr, int64_t size)
{
	return PHYSFS_readBytes((PHYSFS_file *)f, ptr, size);
}

void *
E_ReadFileBlob(File f, int64_t *size, bool transient)
{
	uint8_t *ret = NULL;
	enum MemoryHeap heap = transient ? MH_Transient : MH_Persistent;

	*size = PHYSFS_fileLength((PHYSFS_file *)f);
	if (!*size)
		return NULL;

	ret = Sys_Alloc(sizeof(char), (size_t)*size, heap);
	if (!ret)
		return NULL;

	if (PHYSFS_readBytes((PHYSFS_file *)f, ret, *size) != *size) {
		free(ret);
		return NULL;
	}

	return ret;
}

char *
E_ReadFileText(File f, int64_t *size, bool transient)
{
	char *ret = NULL;
	enum MemoryHeap heap = transient ? MH_Transient : MH_Persistent;

	*size = PHYSFS_fileLength((PHYSFS_file *)f) - PHYSFS_tell((PHYSFS_file *)f);
	if (!*size)
		return NULL;

	ret = Sys_Alloc(sizeof(char), (size_t)*size + 1, heap);
	if (!ret)
		return NULL;

	if (PHYSFS_readBytes((PHYSFS_file *)f, ret, *size) != *size) {
		free(ret);
		return NULL;
	}

	ret[*size - 1] = 0x0;

	return ret;
}

char *
E_FGets(File f, char *buff, int64_t max)
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

		if (c == '\n') {
			*p = 0x0;
			return buff;
		}

		*p = c;
		++p;
	}
	while (1);
}

int64_t
E_WriteFile(File f, void *ptr, int64_t size)
{
	return PHYSFS_writeBytes((PHYSFS_file *)f, ptr, size);
}

int64_t
E_FTell(File f)
{
	return PHYSFS_tell((PHYSFS_file *)f);
}

int64_t
E_FSeek(File f, int64_t offset, FileSeekStart whence)
{
	int64_t dest = offset;

	if (whence == IO_SEEK_CUR)
		dest += PHYSFS_tell((PHYSFS_file *)f);
	else if (whence == IO_SEEK_END)
		dest = PHYSFS_fileLength((PHYSFS_file *)f) - offset;

	return PHYSFS_seek((PHYSFS_file *)f, dest);
}

int64_t
E_FileLength(File f)
{
	return PHYSFS_fileLength((PHYSFS_file *)f);
}

bool
E_FEof(File f)
{
	return PHYSFS_eof((PHYSFS_file *)f);
}

bool
E_FileExists(const char *path)
{
	return PHYSFS_exists(path);
}

void
E_CloseFile(File file)
{
	PHYSFS_close((PHYSFS_file *)file);
}

bool
E_Mount(const char *path, const char *point)
{
	if (!PHYSFS_mount(path, point, 0)) {
		Sys_LogEntry(IO_MODULE, LOG_CRITICAL,
			L"Failed to mount directory [%S] -> [%S]: %S", path, point,
			PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return false;
	}

	return true;
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
E_FileStream(const char *path, FileOpenMode mode, struct Stream *stm)
{
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

	return true;
}

bool
E_MemoryStream(void *buff, uint64_t size, struct Stream *stm)
{
	return false;
}

void
E_CloseStream(struct Stream *stm)
{
	if (stm->type == ST_File)
		E_CloseFile(stm->f);
	else if (stm->type == ST_MappedFile)
		E_UnmapFile(stm->ptr, stm->size);

	memset(stm, 0x0, sizeof(*stm));
}

bool
E_InitIOSystem(const char *argv0)
{
	const PHYSFS_ArchiveInfo **i = NULL;

	if (!PHYSFS_init(argv0)) {
		Sys_LogEntry(IO_MODULE, LOG_CRITICAL, L"Failed to initialize I/O subsystem: %S", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return false;
	}

/*	log_entry(IO_MODULE, LOG_DEBUG, "Data directory [%s]", data_dir);

	if (sys_config_get_bool("load_loose_files", true)) {
		if (!PHYSFS_mount(data_dir, "/", 0))
			log_entry(IO_MODULE, LOG_CRITICAL,
				"Failed to open Data directory: %s",
				PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	}*/

	if (!PHYSFS_mount(E_GetCVarStr(L"Engine_DataDir", "Data")->str, "/", 0))
		return false;

	for (i = PHYSFS_supportedArchiveTypes(); *i != NULL; i++)
		Sys_LogEntry(IO_MODULE, LOG_DEBUG, L"Supported archive: [%S], which is [%S].", (*i)->extension, (*i)->description);

	/*if (!PHYSFS_mountMemory(engine_res, engine_res_size, 0,
		"engine_res.zip", "/system", 0))
		Sys_LogEntry(IO_MODULE, LOG_CRITICAL,
			L"Failed to load builtin resources: %s",
			PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));*/

/*	rt_string write_dir;

	rt_string_init(&write_dir, 2048);
	sys_directory_path(USRDIR_DOCUMENTS, write_dir.data, write_dir.size, true);
	write_dir.length = strlen(write_dir.data);

	rt_string_append_format(&write_dir, "%s%s", PHYSFS_getDirSeparator(),
		e_app_module ? e_app_module->app_name : "NekoEngine");

	if (sys_directory_exists(write_dir.data) < 0) {
		sys_create_directory(write_dir.data);

		rt_string_append(&write_dir, "cache");
		sys_create_directory(write_dir.data);
	}

	PHYSFS_setWriteDir(write_dir.data);
	PHYSFS_addToSearchPath(write_dir.data, 1);

	rt_string_release(&write_dir);*/

	return true;
}

void
E_TermIOSystem(void)
{
	if (!PHYSFS_unmount("engine_res.zip"))
		Sys_LogEntry(IO_MODULE, LOG_DEBUG, L"Failed to unmount engine_res.zip");

	PHYSFS_deinit();
}
