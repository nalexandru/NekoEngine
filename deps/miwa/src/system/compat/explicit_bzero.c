/* Misaki Project - Common Library
 *
 * explicit_bzero.c
 * Created on: Jun 30, 2014
 * Author: Alexandru Naiman
 *
 * Provides utility and compatibility functions used by Misaki components
 *
 * ----------------------------------------------------------------------------------
 * Originally written by Matthew Dempsky. Released into public domain.
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2012-2018, Alexandru Naiman <alexandru dot naiman at icloud dot com>
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

#include <string.h>

#include <system/compat.h>

#if !defined(HAVE_EXPLICIT_BZERO) || (HAVE_EXPLICIT_BZERO == 0)

#ifndef __OpenBSD__

#if __clang__
	/*
	 * http://clang.llvm.org/docs/LanguageExtensions.html#feature-checking-macros
	 * http://lists.cs.uiuc.edu/pipermail/llvmdev/2013-April/061527.html
	 */
	#define ATTRIBUTE_WEAK __attribute__((weak))
	#if __has_attribute(noinline)
		#define NOINLINE __attribute__((noinline))
	#else
		#define NOINLINE
		#warning "clang that supports noinline is required. explicit_bzero may not function as intended"
	#endif
#elif __GNUC__
	/*
	 * http://gcc.gnu.org/onlinedocs/gcc/Function-Specific-Option-Pragmas.html
	 * http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
	 */
	#define ATTRIBUTE_WEAK __attribute__((weak))
	#if __GNUC__ > 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ > 4 )
		#if !defined(__sparc__)
			#pragma GCC push_options
			#pragma GCC optimize ( "-O0" )			
		#endif
		#define NOINLINE __attribute__((noinline))		
	#else
		#define NOINLINE
		#warning "gcc >= 4.4 is required. explicit_bzero may not function as intended"
	#endif
#else
	#define NOINLINE
	#define ATTRIBUTE_WEAK
	#ifndef _MSC_VER	
		#warning "Unrecognized compiler. explicit_bzero may not function as intended"
	#endif
#endif

ATTRIBUTE_WEAK void
__explicit_bzero_hook(void *mem, size_t n)
{
}

NOINLINE void explicit_bzero(void *mem, size_t n)
{
	volatile char *p = mem;

	while(n--)
		*p++=0x0;

	__explicit_bzero_hook(mem, n);
}

#if !defined(__sparc) && (!defined(__clang__) && !defined(_MSC_VER))
	#pragma GCC pop_options
#endif

#endif /* __OpenBSD__ */

#endif /* HAVE_EXPLICIT_BZERO */
