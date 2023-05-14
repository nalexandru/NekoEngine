#ifndef NE_RUNTIME_RTDEFS_H
#define NE_RUNTIME_RTDEFS_H

#include <limits.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RT_DEF_INC	20
#define RT_NOT_FOUND	(size_t)-1
#define RT_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#ifndef SIZE_MAX
#	define SIZE_MAX (4294967295U)
#endif

#ifdef __cplusplus
#	define RT_CEXPR constexpr
#else
#	define RT_CEXPR
#endif

#define RT_HASH(x) Rt_HashLiteral(x)

typedef int32_t(*RtCmpFunc)(const void *item, const void *data);
typedef int32_t(*RtSortFunc)(const void *item, const void *data);

static inline RT_CEXPR const size_t
_Rt_CalcGrowSize(size_t size, size_t elem_size, size_t min_size)
{
	const size_t byte_size = size * elem_size;
	if (size > SIZE_MAX - (byte_size / 2))
		return min_size;

	const size_t geom = size + size / 2;
	if (geom < min_size)
		return min_size;

	return geom;
}

static inline RT_CEXPR const uint64_t
Rt_HashString(const char *str)
{
	uint64_t hash = 0;

	while (*str) {
		hash += *str++;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

static inline RT_CEXPR const uint64_t
Rt_HashMemory(const uint8_t *mem, size_t len)
{
	uint64_t hash = 0;

	for (size_t i = 0; i < len; ++i) {
		hash += mem[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

static inline int32_t
Rt_I32CmpFunc(const void *item, const void *data)
{
	return *((int32_t *)item) - *((int32_t *)data);
}

static inline int32_t
Rt_U32CmpFunc(const void *item, const void *data)
{
	const uint32_t *a = (uint32_t *)item, *b = (uint32_t *)data;
	if (*a > *b)
		return 1;
	else if (*a < *b)
		return -1;
	else
		return 0;
}

static inline int32_t
Rt_U64CmpFunc(const void *item, const void *data)
{
	const uint64_t *a = (uint64_t *)item, *b = (uint64_t *)data;
	if (*a > *b)
		return 1;
	else if (*a < *b)
		return -1;
	else
		return 0;
}

#ifdef __cplusplus

static inline consteval uint64_t
Rt_HashLiteral(const char *str)
{
	uint64_t hash = 0;

	while (*str) {
		hash += *str++;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

}
#else
#	define Rt_HashLiteral Rt_HashString
#endif

#endif /* NE_RUNTIME_RTDEFS_H */

/* NekoEngine
 *
 * RtDefs.h
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
