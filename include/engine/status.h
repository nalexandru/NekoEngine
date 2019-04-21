/* NekoEngine
 *
 * status.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Status Codes
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

#ifndef _NE_ENGINE_STATUS_H_
#define _NE_ENGINE_STATUS_H_

typedef enum ne_status
{
	NE_OK = 0,
	NE_FAIL = 1,
	NE_INVALID_ARGS = 2,
	NE_LIBRARY_LOAD_FAIL = 3,
	NE_INVALID_GFX_LIB = 4,
	NE_NO_PIXEL_FMT = 5,
	NE_GFX_CTX_CREATE_FAIL = 6,
	NE_GFX_GL_LOAD_FAIL = 7,
	NE_WIN_CLS_CREATE_FAIL = 8,
	NE_WIN_CREATE_FAIL = 9,
	NE_SND_DEV_OPEN_FAIL = 10,
	NE_SND_CTX_CREATE_FAIL = 11,
	NE_NO_DISPLAY = 12,
	NE_ALREADY_EXISTS = 13,
	NE_IO_FAIL = 14,
	NE_ALLOC_FAIL = 15,
	NE_OPEN_READ_FAIL = 16,
	NE_OPEN_WRITE_FAIL = 17,
	NE_TOO_MANY_COMPONENTS = 18,
	NE_API_VERSION_MISMATCH = 19,
	NE_NO_MEMORY = 20,
	NE_INVALID_APP_LIB = 21,
	NE_GFX_DEV_CREATE_FAIL = 22,
	NE_INVALID_HEADER = 23,
	NE_MAP_FAIL = 24,
	NE_NOT_REGISTERED = 25,
	NE_FORMAT_UNSUPPORTED = 26,
	NE_INVALID_MESH = 27,
	NE_ABORT_START = 28
} ne_status;

#endif /* _NE_ENGINE_STATUS_H_ */

