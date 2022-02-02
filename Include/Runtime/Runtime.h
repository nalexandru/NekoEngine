#ifndef _NE_RUNTIME_RUNTIME_H_
#define _NE_RUNTIME_RUNTIME_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Runtime/RtDefs.h>
#include <Runtime/Array.h>
#include <Runtime/Queue.h>

#include <System/System.h>

static inline char *
Rt_SkipWhitespace(char *str)
{
	while (*str) {
		if (isspace(*str))
			++str;
		else
			break;
	}
	return str;
}

static inline char *
Rt_StrDup(const char *str, enum NeMemoryHeap mh)
{
	size_t len = strlen(str);
	char *ret = Sys_Alloc(sizeof(*ret), len + 1, mh);
	strncpy(ret, str, len);
	return ret;
}

static inline char *
Rt_TransientStrDup(const char *str)
{
	return Rt_StrDup(str, MH_Transient);
}

#endif /* _NE_RUNTIME_RUNTIME_H_ */
