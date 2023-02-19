#ifndef _NE_SYSTEM_ENDIAN_H_
#define _NE_SYSTEM_ENDIAN_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline bool
Sys_BigEndian(void)
{
	union
	{
		uint32_t i;
		char c[4];
	} bint = { 0x01020304 };

	return (bint.c[0] == 1); 
}

static inline int16_t
Sys_SwapInt16(int16_t val)
{
	return (val << 8) | ((val >> 8) & 0xFF);
}

static inline int32_t
Sys_SwapInt32(int32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF); 
	return (val << 16) | ((val >> 16) & 0xFFFF);
}

static inline int64_t
Sys_SwapInt64(int64_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF); 
	return (val << 16) | ((val >> 16) & 0xFFFF);
}

static inline uint16_t
Sys_SwapUint16(uint16_t val)
{
	return (val << 8) | (val >> 8);
}

static inline uint32_t
Sys_SwapUint32(uint32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

static inline uint64_t
Sys_SwapUint64(uint64_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

static inline float
Sys_SwapFloat(float val)
{
	uint32_t *ptr = (uint32_t *)&val;
	*ptr = Sys_SwapUint32(*ptr);
	return val;
}

#ifdef __cplusplus
}
#endif

#endif /* _NE_SYSTEM_ENDIAN_H_ */

/* NekoEngine
 *
 * Endian.h
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
