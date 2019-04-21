/* NekoEngine
 *
 * meta.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Asset Metadata Helper Functions
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

#include <engine/asset.h>

#define META_RD_BUFF_SZ		10

#define meta_read_proc(name, type, convert_func)			\
ne_status								\
name(									\
	const char *str,						\
	size_t len,							\
	type *out,							\
	uint32_t count)							\
{									\
	char c, buff[META_RD_BUFF_SZ];					\
	uint32_t read = 0, i = 0, i_buff = 0;				\
									\
	memset(buff, 0x0, sizeof(buff));				\
									\
	while (read < count && i < len) {				\
		while ((c = str[i++]) != ',' &&				\
			c != 0x0 &&					\
			i_buff < META_RD_BUFF_SZ &&			\
			i < len)					\
			buff[i_buff++] = c;				\
									\
		out[read] = (type)convert_func(buff);			\
		memset(buff, 0x0, sizeof(buff));			\
									\
		i_buff = 0;						\
		++read;							\
		++i;							\
	}								\
									\
	return read == count ? NE_OK : NE_FAIL;				\
}

meta_read_proc(meta_read_floats, float, atof)
meta_read_proc(meta_read_doubles, double, atof)
