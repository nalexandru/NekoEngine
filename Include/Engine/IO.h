#ifndef NE_ENGINE_IO_H
#define NE_ENGINE_IO_H

#include <stdio.h>
#include <string.h>

#include <Engine/Types.h>
#include <System/Memory.h>

#ifdef __cplusplus
extern "C" {
#endif

enum NeFileOpenMode
{
	IO_READ,
	IO_WRITE,
	IO_APPEND
};

enum NeFileSeekStart 
{
	IO_SEEK_SET,
	IO_SEEK_CUR,
	IO_SEEK_END
};

enum NeStreamType
{
	ST_Closed = 0,
	ST_Memory,
	ST_File,
	ST_MappedFile
};

enum NeWriteDirectory
{
	WD_Data,
	WD_Save,
	WD_Temp,
	WD_Config
};

struct NeStream
{
	uint8_t *ptr;
	uint64_t pos, size;
	NeFile f;
	enum NeStreamType type;
	bool open;
};

NeFile		  E_OpenFile(const char *path, enum NeFileOpenMode mode);
void		 *E_MapFile(const char *path, enum NeFileOpenMode mode, uint64_t *size);
void		  E_UnmapFile(const void *ptr, uint64_t size);
int64_t		  E_ReadFile(NeFile f, void *ptr, int64_t size);
void		 *E_ReadFileBlob(NeFile f, int64_t *size, bool transient);
char		 *E_ReadFileText(NeFile f, int64_t *size, bool transient);
char		 *E_FGets(NeFile f, char *buff, int64_t max);
int64_t		  E_WriteFile(NeFile f, const void *ptr, int64_t size);
int64_t		  E_FTell(NeFile f);
int64_t		  E_FSeek(NeFile f, int64_t offset, enum NeFileSeekStart whence);
int64_t		  E_FileLength(NeFile f);
bool		  E_FEof(NeFile f);
bool		  E_FileExists(const char *path);
void		  E_CloseFile(NeFile file);
bool		  E_Mount(const char *path, const char *point);
bool		  E_MountMemory(const char *name, const void *ptr, uint64_t size, const char *point);
bool		  E_Unmount(const char *name);
const char	**E_ListFiles(const char *dir);
bool		  E_IsDirectory(const char *path);
void		  E_FreeFileList(const char **list);
void		  E_ProcessFiles(const char *path, const char *ext, bool recurse, void (*cb)(const char *));

void		 *E_WatchDirectory(const char *path, enum NeFSEvent mask, NeDirWatchCallback callback, void *ud);
void		  E_RemoveWatch(void *watch);

bool		  E_EnableWrite(enum NeWriteDirectory wd);
void		  E_DisableWrite(void);
bool		  E_CreateDirectory(const char *name);

bool		  E_FileStream(const char *path, enum NeFileOpenMode mode, struct NeStream *stm);
bool		  E_MappedFileStream(const char *path, enum NeFileOpenMode mode, struct NeStream *stm);
bool		  E_MemoryStream(void *buff, uint64_t size, struct NeStream *stm);
void		  E_CloseStream(struct NeStream *stm);

bool		  E_InitIOSystem(void);
void		  E_TermIOSystem(void);

// Inline functions
static inline int64_t
E_StreamTell(const struct NeStream *stm)
{
	if (stm->ptr)
		return stm->pos;
	else if (stm->f)
		return E_FTell(stm->f);
	else
		return 0;
}

static inline int64_t
E_SeekStream(struct NeStream *stm, int64_t offset, enum NeFileSeekStart whence)
{
	if (stm->ptr) {
		switch (whence) {
		case IO_SEEK_SET: stm->pos = offset; break;
		case IO_SEEK_CUR: stm->pos += offset; break;
		case IO_SEEK_END: stm->pos = stm->size - offset; break;
		}

		return 0;
	} else if (stm->f) {
		return E_FSeek(stm->f, offset, whence);
	} else {
		return -1;
	}
}

static inline int64_t
E_StreamLength(const struct NeStream *stm)
{
	if (stm->ptr)
		return stm->size;
	else if (stm->f)
		return E_FileLength(stm->f);
	else
		return 0;
}

static inline bool
E_EndOfStream(const struct NeStream *stm)
{
	if (stm->ptr)
		return stm->pos == stm->size;
	else if (stm->f)
		return E_FEof(stm->f);
	else
		return true;
}

static inline int64_t
E_ReadStream(struct NeStream *stm, void *ptr, int64_t size)
{
	if (stm->ptr) {
		memmove(ptr, stm->ptr + stm->pos, (size_t)size);
		stm->pos += size;
		return size;
	} else if (stm->f) {
		return E_ReadFile(stm->f, ptr, size);
	} else {
		return -1;
	}
}

static inline char *
E_ReadStreamLine(struct NeStream *stm, char *ptr, int64_t size)
{
	char *ret = NULL;

	if (stm->ptr) {
		if (stm->pos == stm->size)
			return NULL;

		ret = ptr;

		while (size && (stm->pos < stm->size)) {
			*ptr = *(stm->ptr + stm->pos++);

			if (*ptr == '\n') {
				*ptr-- = 0x0;
				if (*ptr == '\r')
					*ptr = 0x0;
				break;
			}

			++ptr;
			--size;
		}
	} else if (stm->f) {
		ret = E_FGets(stm->f, ptr, size);
	}

	return ret;
}

static inline void *
E_ReadStreamBlob(struct NeStream *stm, enum NeMemoryHeap heap)
{
	size_t size = (size_t)E_StreamLength(stm);
	if (!stm->open || !size)
		return NULL;

	void *blob = Sys_Alloc(size, 1, heap);

	if (stm->ptr) {
		stm->pos = 0;
		memmove(blob, stm->ptr + stm->pos, size);
		stm->pos += size;
	} else if (stm->f) {
		E_FSeek(stm->f, 0, IO_SEEK_SET);
		E_ReadFile(stm->f, blob, size);
	} else {
		Sys_Free(blob);
		return NULL;
	}

	return blob;
}

static inline int64_t
E_WriteStream(struct NeStream *stm, const void *ptr, int64_t size)
{
	if (stm->ptr) {
		memmove(stm->ptr + stm->pos, ptr, (size_t)size);
		stm->pos += size;
		return size;
	} else if (stm->f) {
		return E_WriteFile(stm->f, ptr, size);
	} else {
		return -1;
	}
}

#ifdef __cplusplus
}
#endif

#endif /* NE_ENGINE_IO_H */

/* NekoEngine
 *
 * IO.h
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
