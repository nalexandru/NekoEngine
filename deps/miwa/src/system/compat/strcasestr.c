/* Mizuki Project - Common Library
 *
 * strcasestr.c
 * Created on: Jul 16, 2014
 * Author: Alexandru Naiman
 *
 * Provides utility and compatibility functions used by Mizuki components
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2012-2017, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.  
 */

#include <system/compat.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#define strncasecmp _strnicmp
#define HAVE_STRCASESTR 1
#endif

#if !defined(HAVE_STRCASESTR) || (HAVE_STRCASESTR == 0)

char *strcasestr(char *a, char *b)
{
	size_t len;
	char c[3];

	snprintf(c, sizeof(c), "%c%c", tolower(*b), toupper(*b));

	for (len = strcspn(a, c); len != strlen(a); len += strcspn(a + len + 1, c) + 1)
		if (strncasecmp(a + len, b, strlen(b)) == 0)
			return a + len;

	return NULL;
}

#endif /* HAVE_STRCASESTR */
