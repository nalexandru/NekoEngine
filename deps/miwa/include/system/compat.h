/* Miwa Portable Runtime
 *
 * compat.h
 * Author: Alexandru Naiman
 *
 * Compatibility shivs
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

#ifndef _MIWA_SYSTEM_COMPAT_H_
#define _MIWA_SYSTEM_COMPAT_H_

#include <string.h>
#include <system/miwa_config.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#define strtok_r strtok_s
	#define mlock VirtualLock
	#define munlock VirtualUnlock

	#if _MSC_VER > 1200
		#define explicit_bzero SecureZeroMemory
		#define HAVE_EXPLICIT_BZERO	1
	#endif

	#ifdef __MINGW32__
		#include <sys/types.h>
		#ifndef PATH_MAX
			#define PATH_MAX MAX_PATH
		#endif
	#else
		typedef DWORD pid_t;
		#define PATH_MAX MAX_PATH 
	#endif
#elif defined(__linux__) && (HAVE_LIBBSD == 1)
        #include <bsd/string.h>
        #include <bsd/stdlib.h>

        void explicit_bzero(void *buf, size_t len);
        void *reallocarray(void *ptr, size_t nmemb, size_t size);
#endif /* WIN32 || WIN64 */

#ifdef _MSC_VER
        #if _MSC_VER <= 1800
                #define INLINE
				#define getpid GetCurrentProcessId
        #else
                #define INLINE inline
        #endif

        #if _MSC_VER <= 1200
                #define socklen_t uint32_t
                #define HAVE_EXPLICIT_BZERO		0
                #define HAVE_VSNPRINTF			0
        #endif
#else
        #define INLINE inline
#endif /* _MSC_VER */

#if defined(_MSC_VER) || defined(_WIN32) || defined(__sun) || defined(__APPLE__)
	#define bswap_16(x)	(((x) << 8) & 0xff00) | (((x) >> 8 ) & 0xff)
	#define bswap_32(x) (((x) << 24) & 0xff000000)  \
					| (((x) << 8) & 0xff0000)   \
					| (((x) >> 8) & 0xff00)     \
					| (((x) >> 24) & 0xff)

#if defined(__sun) || defined(__APPLE__)
	#define bswap_64(x) ((((x) & 0xff00000000000000ull) >> 56) \
					| (((x) & 0x00ff000000000000ull) >> 40) \
					| (((x) & 0x0000ff0000000000ull) >> 24) \
					| (((x) & 0x000000ff00000000ull) >> 8) \
					| (((x) & 0x00000000ff000000ull) << 8) \
					| (((x) & 0x0000000000ff0000ull) << 24) \
					| (((x) & 0x000000000000ff00ull) << 40) \
					| (((x) & 0x00000000000000ffull) << 56))
#else /* defined(__sun) || defined(__APPLE__) */
	#define bswap_64(x) ((((x) & 0xff00000000000000ui64) >> 56) \
					| (((x) & 0x00ff000000000000ui64) >> 40) \
					| (((x) & 0x0000ff0000000000ui64) >> 24) \
					| (((x) & 0x000000ff00000000ui64) >> 8) \
					| (((x) & 0x00000000ff000000ui64) << 8) \
					| (((x) & 0x0000000000ff0000ui64) << 24) \
					| (((x) & 0x000000000000ff00ui64) << 40) \
					| (((x) & 0x00000000000000ffui64) << 56))
#endif /* __sun */

#elif defined(__NetBSD__)
	#include <sys/types.h>
	#include <machine/bswap.h>

	#define bswap_16 bswap16
	#define bswap_32 bswap32
	#define bswap_64 bswap64
#else
	#include <byteswap.h>
#endif /* _MSC_VER */

#if HAVE_STRCASESTR == 0
	char *strcasestr(char *a, char *b);
#endif /* HAVE_STRCASESTR */

#if HAVE_STRLCAT == 0
	size_t strlcat(char *dst, const char *src, size_t dsize);
#endif /* HAVE_STRLCAT */

#if HAVE_STRLCPY == 0
	size_t strlcpy(char *dst, const char *src, size_t dsize);
#endif /* HAVE_STRLCPY */

#if HAVE_STRNLEN == 0
	size_t strnlen(const char *str, size_t maxlen);
#endif /* HAVE_STRNLEN */

#if HAVE_SNPRINTF == 0
	int snprintf(char *, size_t, const char *, /*args*/ ...);
#else /* HAVE_SNPRINTF */
	#include <stdio.h>
#endif /* HAVE_SNPRINTF */

#if HAVE_VSNPRINTF == 0
	#ifndef vsnprintf
		#include <stdarg.h>
		int vsnprintf(char *, size_t, const char *, va_list);
	#endif /* vsnprintf */
#endif /* HAVE_VSNPRINTF */

#if HAVE_REALLOCARRAY == 0
	void *reallocarray(void *optr, size_t nmemb, size_t size);
#endif /* HAVE_REALLOCARRAY */

#if HAVE_EXPLICIT_BZERO == 0
	void explicit_bzero(void *mem, size_t n);
#endif /* HAVE_EXPLICIT_BZERO */

#ifndef PATH_MAX
	#define PATH_MAX	1024
#endif /* PATH_MAX */

#ifdef __cplusplus
}
#endif

#endif /* _MIWA_SYSTEM_COMPAT_H_ */
