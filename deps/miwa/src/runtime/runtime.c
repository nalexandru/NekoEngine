/* Miwa Portable Runtime
 *
 * runtime.c
 * Author: Alexandru Naiman
 *
 * Runtime
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (c) 2018-2019, Alexandru Naiman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _DEFAULT_SOURCE

#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <system/compat.h>
#include <runtime/runtime.h>

size_t
_rt_calc_grow_size(size_t size,
	size_t elem_size,
	size_t min_size)
{
	size_t byte_size = size * elem_size;
	size_t geom = 0;

	if (size > SIZE_MAX - (byte_size / 2))
		return min_size;

	geom = size + size / 2;
	if (geom < min_size)
		return min_size;

	return geom;
}

int
rt_is_numeric(const char *str)
{
	size_t len = 0, i = 0;

	if ((str == NULL) || ((len = strlen(str)) == 0))
		return 0;

	for (i = 0; i < len; i++)
	{
		if (!isdigit(str[i]))
			return 0;
	}

	return 1;
}

void
rt_swap_int16(int16_t *val)
{
	*val = (*val << 8) | ((*val >> 8) & 0xFF);
}

void
rt_swap_int32(int32_t *val)
{
	*val = ((*val << 8) & 0xFF00FF00) | ((*val >> 8) & 0xFF00FF); 
	*val = (*val << 16) | ((*val >> 16) & 0xFFFF);
}

void
rt_swap_int64(int64_t *val)
{
	*val = ((*val << 8) & 0xFF00FF00) | ((*val >> 8) & 0xFF00FF); 
	*val = (*val << 16) | ((*val >> 16) & 0xFFFF);
}

void
rt_swap_uint16(uint16_t *val)
{
	*val = (*val << 8) | (*val >> 8);
}

void
rt_swap_uint32(uint32_t *val)
{
	*val = ((*val << 8) & 0xFF00FF00) | ((*val >> 8) & 0xFF00FF);
	*val = (*val << 16) | (*val >> 16);
}

void
rt_swap_uint64(uint64_t *val)
{
	*val = ((*val << 8) & 0xFF00FF00) | ((*val >> 8) & 0xFF00FF);
	*val = (*val << 16) | (*val >> 16);
}
