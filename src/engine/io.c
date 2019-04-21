/* NekoEngine
 *
 * io.c
 * Author: Alexandru Naiman
 *
 * NekoEngine I/O Subsystem
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 */

#include <string.h>
#include <stdlib.h>

#include <physfs.h>

#include <system/log.h>
#include <system/config.h>

#include <engine/io.h>

#define IO_MODULE "I/O"

extern unsigned char engine_res[];
extern unsigned int engine_res_size;

ne_file *
io_open(const char *path,
	ne_file_open_mode mode)
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
		log_entry(IO_MODULE, LOG_CRITICAL,
			"Failed to open file [%s], %s", path,
			PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

	return (ne_file *)f;
}

int64_t
io_read(ne_file *f,
	void *ptr,
	int64_t size)
{
	return PHYSFS_readBytes((PHYSFS_file *)f, ptr, size);
}

void *
io_read_blob(
	ne_file *f,
	int64_t *size)
{
	uint8_t *ret = NULL;

	*size = PHYSFS_fileLength((PHYSFS_file *)f);

	if ((ret = malloc(*size)) == NULL)
		return NULL;

	if (PHYSFS_readBytes((PHYSFS_file *)f, ret, *size) != *size) {
		free(ret);
		return NULL;
	}

	return ret;
}

char *
io_read_text(
	ne_file *f,
	int64_t *size)
{
	char *ret = NULL;

	*size = PHYSFS_fileLength((PHYSFS_file *)f);

	if ((ret = malloc(*size + 1)) == NULL)
		return NULL;

	if (PHYSFS_readBytes((PHYSFS_file *)f, ret, *size) != *size) {
		free(ret);
		return NULL;
	}

	ret[*size - 1] = 0x0;

	return ret;
}

char *
io_gets(ne_file *f,
	char *buff,
	int64_t max)
{
	char c;
	char *p = buff;

	if (!f)
		return NULL;

	memset(buff, 0x0, max);

	do {
		if (io_read(f, &c, 1) <= 0)
			return NULL;
	} while (c == ' ' || c == '\t');

	*p = c;
	++p;

	do {
		if (io_read(f, &c, 1) <= 0) {
			if (io_eof(f)) {
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
	} while (1);
}

int64_t
io_write(
	ne_file *f,
	void *ptr,
	int64_t size)
{
	return PHYSFS_writeBytes((PHYSFS_file *)f, ptr, size);
}

int64_t
io_tell(ne_file *f)
{
	return PHYSFS_tell((PHYSFS_file *)f);
}

int64_t
io_seek(ne_file *f,
	int64_t offset,
	ne_file_seek_start whence)
{
	int64_t dest = offset;

	if (whence == IO_SEEK_CUR)
		dest += PHYSFS_tell((PHYSFS_file *)f);
	else if (whence == IO_SEEK_END)
		dest = PHYSFS_fileLength((PHYSFS_file *)f) - offset;

	return PHYSFS_seek((PHYSFS_file *)f, dest);
}

int64_t
io_length(ne_file *f)
{
	return PHYSFS_fileLength((PHYSFS_file *)f);
}

bool
io_eof(ne_file *f)
{
	return PHYSFS_eof((PHYSFS_file *)f);
}

bool
io_exists(const char *path)
{
	return PHYSFS_exists(path);
}

void
io_close(ne_file *file)
{
	PHYSFS_close((PHYSFS_file *)file);
}

ne_status
io_sys_init(const char *argv0)
{
	if (!PHYSFS_init(argv0)) {
		log_entry(IO_MODULE, LOG_CRITICAL,
			"Failed to initialize I/O subsystem: %s",
			PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return NE_FAIL;
	}

	if (sys_config_get_bool("load_loose_files", true)) {
		if (!PHYSFS_mount("data", "/", 0))
			log_entry(IO_MODULE, LOG_CRITICAL,
			"Failed to open Data directory: %s",
			PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	}

	for (const PHYSFS_ArchiveInfo **i = PHYSFS_supportedArchiveTypes();
			*i != NULL; i++)
		log_entry(IO_MODULE, LOG_DEBUG,
				"Supported archive: [%s], which is [%s].\n",
				(*i)->extension, (*i)->description);

	if (!PHYSFS_mountMemory(engine_res, engine_res_size, NULL,
				"engine_res.zip", "/system", 0))
			log_entry(IO_MODULE, LOG_CRITICAL,
			"Failed to load builtin resources: %s",
			PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

	for (char **i = PHYSFS_enumerateFiles("/system");
			*i != NULL; i++)
		log_entry(IO_MODULE, LOG_DEBUG,
				"[%s]\n",
				*i);

	for (char **i = PHYSFS_enumerateFiles("/system");
			*i != NULL; i++)
		log_entry(IO_MODULE, LOG_DEBUG,
				"[%s]\n",
				*i);

	PHYSFS_setWriteDir("data");

	return NE_OK;
}

void
io_sys_release(void)
{
	PHYSFS_deinit();
}
