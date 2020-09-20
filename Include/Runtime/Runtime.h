#ifndef _RT_RUNTIME_H_
#define _RT_RUNTIME_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Runtime/RtDefs.h>
#include <Runtime/Array.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline wchar_t *
Rt_MbsToWcs(const char *str)
{
	size_t len = strlen(str);

	wchar_t *ret = (wchar_t *)calloc(len + 1, sizeof(*ret));
	(void)mbstowcs(ret, str, len);

	return ret;
}

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

#ifdef __cplusplus
}
#endif

#endif /* _RT_RUNTIME_H_ */