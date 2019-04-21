/* Miwa Portable Runtime
 *
 * platform.h
 * Author: Alexandru Naiman
 *
 * Platform Detection
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

#ifndef _MIWA_SYSTEM_PLATFORM_H_
#define _MIWA_SYSTEM_PLATFORM_H_

#if defined(_WIN64)
	#define SYS_PLATFORM_WIN64
	#define SYS_PLATFORM_WINDOWS
	#define SYS_ARCH_X86_64
	#define SYS_64BIT
	#if defined(__MINGW64__)
		#define SYS_PLATFORM_MINGW
	#endif
#elif defined(_WIN32)
	#define SYS_PLATFORM_WIN32
	#define SYS_PLATFORM_WINDOWS
	#define SYS_ARCH_X86
	#if defined(__MINGW32__)
		#define SYS_PLATFORM_MINGW
	#endif
#elif defined(__linux__)
	#define SYS_PLATFORM_LINUX
	#define SYS_PLATFORM_UNIX
	#define SYS_PLATFORM_X11
	#if defined(__arm__)
		#ifdef __LP64__
			#define SYS_PLATFORM_LINUX_ARM64
			#define SYS_ARCH_ARM64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_LINUX_ARM
			#define SYS_ARCH_ARM
		#endif
	#elif defined(__sparc)
		#ifdef __LP64__
			#define SYS_PLATFORM_LINUX_SPARC64
			#define SYS_ARCH_SPARC64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_LINUX_SPARC
			#define SYS_ARCH_SPARC
		#endif
	#elif defined(__mips__)
		#ifdef __LP64__
			#define SYS_PLATFORM_LINUX_MIPS64
			#define SYS_ARCH_MIPS64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_LINUX_MIPS
			#define SYS_ARCH_MIPS
		#endif
	#elif defined(__powerpc__)
		#ifdef __LP64__
			#define SYS_PLATFORM_LINUX_PPC64
			#define SYS_ARCH_PPC64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_LINUX_PPC
			#define SYS_ARCH_PPC
		#endif
	#else
		#ifdef __LP64__
			#define SYS_PLATFORM_LINUX_X86_64
			#define SYS_ARCH_X86_64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_LINUX_X86
			#define SYS_ARCH_X86
		#endif
	#endif
#elif defined(__APPLE__) && defined(__MACH__)
	#define SYS_PLATFORM_UNIX
	#define SYS_PLATFORM_APPLE
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR == 1
		#define SYS_PLATFORM_IOS
		#define SYS_PLATFORM_IOS_SIM
		#define SYS_ARCH_X86_64
		#define SYS_64BIT
		#define SYS_DEVICE_MOBILE
	#elif TARGET_OS_IPHONE == 1
		#define SYS_PLATFORM_IOS
		#define SYS_DEVICE_MOBILE
		#ifdef __LP64__
			#define SYS_ARCH_ARM64
			#define SYS_64BIT
		#else
			#define SYS_ARCH_ARM
		#endif
	#else
		#define SYS_PLATFORM_MAC
		#ifdef __LP64__
			#define SYS_ARCH_X86_64
			#define SYS_64BIT
		#else
			#define SYS_ARCH_X86
		#endif
	#endif
#elif defined(__QNX__)
	#define SYS_PLATFORM_UNIX
	#define SYS_DEVICE_MOBILE
	#ifdef __arm__
		#define SYS_PLATFORM_BB10
		#ifdef __LP64__
			#define SYS_ARCH_ARM64
			#define SYS_64BIT
		#else
			#define SYS_ARCH_ARM
		#endif
	#else
		#define SYS_PLATFORM_BB10
		#ifdef __LP64__
			#define SYS_ARCH_X86_64
			#define SYS_64BIT
		#else
			#define SYS_ARCH_ARM
		#endif
	#endif
#elif defined(__FreeBSD__)
	#define SYS_PLATFORM_UNIX
	#define SYS_PLATFORM_FREEBSD
	#define SYS_PLATFORM_X11
	#if defined(__arm__)
		#ifdef __LP64__
			#define SYS_PLATFORM_FREEBSD_ARM64
			#define SYS_ARCH_ARM64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_FREEBSD_ARM
			#define SYS_ARCH_ARM
		#endif
	#else
		#ifdef __LP64__
			#define SYS_PLATFORM_FREEBSD_X86_64
			#define SYS_ARCH_X86_64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_FREEBSD_X86
			#define SYS_ARCH_X86
		#endif
	#endif
#elif defined(__DragonFly__)
	#define SYS_PLATFORM_UNIX
	#define SYS_PLATFORM_DRAGONFLY
	#define SYS_PLATFORM_X11
	#if defined(__arm__)
		#ifdef __LP64__
			#define SYS_PLATFORM_DRAGONFLY_ARM64
			#define SYS_ARCH_ARM64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_DRAGONFLY_ARM
			#define SYS_ARCH_ARM
		#endif
	#else
		#ifdef __LP64__
			#define SYS_PLATFORM_DRAGONFLY_X86_64
			#define SYS_ARCH_X86_64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_DRAGONFLY_X86
			#define SYS_ARCH_X86
		#endif
	#endif
#elif defined(__NetBSD__)
	#define SYS_PLATFORM_UNIX
	#define SYS_PLATFORM_NETBSD
	#define SYS_PLATFORM_X11
	#if defined(__arm__)
		#ifdef __LP64__
			#define SYS_PLATFORM_NETBSD_ARM64
			#define SYS_ARCH_ARM64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_NETBSD_ARM
			#define SYS_ARCH_ARM
		#endif
	#else
		#ifdef __LP64__
			#define SYS_PLATFORM_NETBSD_X86_64
			#define SYS_ARCH_X86_64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_NETBSD_X86
			#define SYS_ARCH_X86
		#endif
	#endif
#elif defined(__OpenBSD__)
	#define SYS_PLATFORM_UNIX
	#define SYS_PLATFORM_OPENBSD
	#define SYS_PLATFORM_X11
	#if defined(__arm__)
		#ifdef __LP64__
			#define SYS_PLATFORM_OPENBSD_ARM64
			#define SYS_ARCH_ARM64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_OPENBSD_ARM
			#define SYS_ARCH_ARM
		#endif
	#else
		#ifdef __LP64__
			#define SYS_PLATFORM_OPENBSD_X86_64
			#define SYS_ARCH_X86_64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_OPENBSD_X86
			#define SYS_ARCH_X86
		#endif
	#endif
#elif defined(sun) || defined(__sun)
	#define SYS_PLATFORM_UNIX
	#define SYS_PLATFORM_SUNOS
	#define SYS_PLATFORM_X11
	#if defined(__sparc)
		#ifdef __LP64__
			#define SYS_PLATFORM_SUNOS_SPARC64
			#define SYS_ARCH_SPARC64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_SUNOS_SPARC
			#define SYS_ARCH_SPARC
		#endif
	#else
		#ifdef __LP64__
			#define SYS_PLATFORM_SUNOS_X86_64
			#define SYS_ARCH_X86_64
			#define SYS_64BIT
		#else
			#define SYS_PLATFORM_SUNOS_X86
			#define SYS_ARCH_X86
		#endif
	#endif
#elif defined(__ANDROID__)
	#define SYS_PLATFORM_ANDROID
	#define SYS_PLATFORM_UNIX
#endif

#if (defined(_WIN32) || defined(_WIN64)) && defined(_MSC_VER) && !defined(MIWA_EMPTY_EXPORT)
	#ifdef MIWA_EXPORT_SYMBOLS
		#define MIWA_EXPORT __declspec(dllexport)
	#else
		#define MIWA_EXPORT __declspec(dllimport)
	#endif
#else
	#define MIWA_EXPORT
#endif

#endif /* _MIWA_SYSTEM_PLATFORM_H_ */
