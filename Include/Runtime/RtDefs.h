#ifndef _NE_RUNTIME_RTDEFS_H_
#define _NE_RUNTIME_RTDEFS_H_

#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#define RT_DEF_INC	20
#define RT_NOT_FOUND	(size_t)-1
#define RT_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#ifndef SIZE_MAX
#	define SIZE_MAX (4294967295U)
#endif

typedef int32_t(*RtCmpFunc)(const void *item, const void *data);
typedef int32_t(*RtSortFunc)(const void *item, const void *data);

static inline size_t
_Rt_CalcGrowSize(size_t size, size_t elem_size, size_t min_size)
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

static inline uint64_t
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

static inline uint64_t
Rt_HashStringW(const wchar_t *str)
{
	uint64_t hash = 0;
	size_t i = 0;
	size_t len = wcslen(str);

	for (i = 0; i < len; ++i) {
		hash += str[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

#endif /* _NE_RUNTIME_RTDEFS_H_ */
