/* NekoEngine
 *
 * io.h
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

#ifndef _NE_ENGINE_IO_H_
#define _NE_ENGINE_IO_H_

#include <stdint.h>
#include <stdbool.h>

#include <engine/status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ne_file_open_mode
{
	IO_READ,
	IO_WRITE,
	IO_APPEND
} ne_file_open_mode;

typedef enum ne_file_seek_start
{
	IO_SEEK_SET,
	IO_SEEK_CUR,
	IO_SEEK_END
} ne_file_seek_start;

typedef void * ne_file;

ne_file		*io_open(const char *path, ne_file_open_mode mode);
int64_t		 io_read(ne_file *f, void *ptr, int64_t size);
void		*io_read_blob(ne_file *f, int64_t *size);
char		*io_read_text(ne_file *f, int64_t *size);
char		*io_gets(ne_file *f, char *buff, int64_t max);
int64_t		 io_write(ne_file *f, void *ptr, int64_t size);
int64_t		 io_tell(ne_file *f);
int64_t		 io_seek(ne_file *f, int64_t offset, ne_file_seek_start whence);
int64_t		 io_length(ne_file *f);
bool		 io_eof(ne_file *f);
bool		 io_exists(const char *path);
void		 io_close(ne_file *file);

#ifdef _NE_ENGINE_INTERNAL_

ne_status	 io_sys_init(const char *argv0);
void		 io_sys_release(void);

#endif /* _NE_ENGINE_INTERNAL_ */

#ifdef __cplusplus
}
#endif

#endif /* _NE_ENGINE_IO_H_ */

