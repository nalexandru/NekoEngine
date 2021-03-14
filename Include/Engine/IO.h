#ifndef _E_IO_H_
#define _E_IO_H_

#include <stdio.h>
#include <string.h>

#include <Engine/Types.h>

typedef enum FileOpenMode
{
	IO_READ,
	IO_WRITE,
	IO_APPEND
} FileOpenMode;

typedef enum FileSeekStart 
{
	IO_SEEK_SET,
	IO_SEEK_CUR,
	IO_SEEK_END
} FileSeekStart;

enum StreamType
{
	ST_Closed = 0,
	ST_Memory,
	ST_File,
	ST_MappedFile
};

enum WriteDirectory
{
	WD_Data,
	WD_Save,
	WD_Temp,
	WD_Config
};

struct Stream
{
	uint8_t *ptr;
	uint64_t pos, size;
	File f;
	enum StreamType type;
};

File		  E_OpenFile(const char *path, FileOpenMode mode);
void		 *E_MapFile(const char *path, FileOpenMode mode, uint64_t *size);
void		  E_UnmapFile(const void *ptr, uint64_t size);
int64_t		  E_ReadFile(File f, void *ptr, int64_t size);
void		 *E_ReadFileBlob(File f, int64_t *size, bool transient);
char		 *E_ReadFileText(File f, int64_t *size, bool transient);
char		 *E_FGets(File f, char *buff, int64_t max);
int64_t		  E_WriteFile(File f, void *ptr, int64_t size);
int64_t		  E_FTell(File f);
int64_t		  E_FSeek(File f, int64_t offset, FileSeekStart whence);
int64_t		  E_FileLength(File f);
bool		  E_FEof(File f);
bool		  E_FileExists(const char *path);
void		  E_CloseFile(File file);
bool		  E_Mount(const char *path, const char *point);
bool		  E_MountMemory(const char *name, const void *ptr, uint64_t size, const char *point);
const char	**E_ListFiles(const char *dir);
bool		  E_IsDirectory(const char *path);
void		  E_FreeFileList(const char **list);
void		  E_ProcessFiles(const char *path, const char *ext, bool recurse, void (*cb)(const char *));

bool		  E_EnableWrite(enum WriteDirectory wd);
void		  E_DisableWrite(void);
bool		  E_CreateDirectory(const char *name);

bool		  E_FileStream(const char *path, FileOpenMode mode, struct Stream *stm);
bool		  E_MemoryStream(void *buff, uint64_t size, struct Stream *stm);
void		  E_CloseStream(struct Stream *stm);

bool		  E_InitIOSystem(void);
void		  E_TermIOSystem(void);

// Inline functions
static inline int64_t
E_ReadStream(struct Stream *stm, void *ptr, int64_t size)
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
E_ReadStreamLine(struct Stream *stm, char *ptr, int64_t size)
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

static inline int64_t
E_WriteStream(struct Stream *stm, void *ptr, int64_t size)
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

static inline int64_t
E_StreamTell(const struct Stream *stm)
{
	if (stm->ptr)
		return stm->pos;
	else if (stm->f)
		return E_FTell(stm->f);
	else
		return 0;
}

static inline int64_t
E_StreamSeek(struct Stream *stm, int64_t offset, FileSeekStart whence)
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
E_StreamLength(const struct Stream *stm)
{
	if (stm->ptr)
		return stm->size;
	else if (stm->f)
		return E_FileLength(stm->f);
	else
		return 0;
}

static inline bool
E_EndOfStream(const struct Stream *stm)
{
	if (stm->ptr)
		return stm->pos == stm->size;
	else if (stm->f)
		return E_FEof(stm->f);
	else
		return true;
}

#endif /* _E_IO_H_ */
