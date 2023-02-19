#ifndef _NE_SYSTEM_PLATFORM_DETECT_H_
#define _NE_SYSTEM_PLATFORM_DETECT_H_

#if defined(_WIN64)
#	define SYS_PLATFORM_WIN64
#	define SYS_PLATFORM_WINDOWS
#	define SYS_ARCH_X86_64
#	define SYS_64BIT
#	if defined(__MINGW64__)
#		define SYS_PLATFORM_MINGW
#	endif
#elif defined(_XBOX)
#	ifdef _XENON
#		define SYS_PLATFORM_XBOX_360
#		define SYS_ARCH_PPC64
#	else
#		define SYS_PLATFORM_XBOX
#		define SYS_ARCH_X86
#	endif
#elif defined(_WIN32)
#	define SYS_PLATFORM_WIN32
#	define SYS_PLATFORM_WINDOWS
#	define SYS_ARCH_X86
#	if defined(__MINGW32__)
#		define SYS_PLATFORM_MINGW
#	endif
#elif defined(__linux__) && !defined(__ANDROID__)
#	define SYS_PLATFORM_LINUX
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_X11
#	if defined(__arm__)
#		ifdef __LP64__
#			define SYS_PLATFORM_LINUX_ARM64
#			define SYS_ARCH_ARM64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_LINUX_ARM
#			define SYS_ARCH_ARM
#		endif
#	elif defined(__ARM_ARCH)
#		define SYS_PLATFORM_LINUX_ARM64
#		define SYS_ARCH_ARM64
#		define SYS_64BIT
#	elif defined(__sparc)
#		ifdef __LP64__
#			define SYS_PLATFORM_LINUX_SPARC64
#			define SYS_ARCH_SPARC64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_LINUX_SPARC
#			define SYS_ARCH_SPARC
#		endif
#	elif defined(__mips__)
#		ifdef __LP64__
#			define SYS_PLATFORM_LINUX_MIPS64
#			define SYS_ARCH_MIPS64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_LINUX_MIPS
#			define SYS_ARCH_MIPS
#		endif
#	elif defined(__powerpc__)
#		ifdef __LP64__
#			define SYS_PLATFORM_LINUX_PPC64
#			define SYS_ARCH_PPC64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_LINUX_PPC
#			define SYS_ARCH_PPC
#		endif
#	elif defined(__hppa__)
#		define SYS_PLATFORM_LINUX_HPPA
#		define SYS_ARCH_HPPA
#	elif defined(__alpha__)
#		define SYS_PLATFORM_LINUX_ALPHA
#		define SYS_ARCH_ALPHA
#		define SYS_64BIT
#	elif defined(__riscv)
#		ifdef __LP64__
#			define SYS_PLATFORM_LINUX_RV64
#			define SYS_ARCH_RV64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_LINUX_RV32
#			define SYS_ARCH_RV32
#		endif
#	else
#		ifdef __LP64__
#			define SYS_PLATFORM_LINUX_X86_64
#			define SYS_ARCH_X86_64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_LINUX_X86
#			define SYS_ARCH_X86
#		endif
#	endif
#elif defined(__APPLE__) && defined(__MACH__)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_APPLE
#	include <TargetConditionals.h>
#	if TARGET_IPHONE_SIMULATOR == 1
#		define SYS_PLATFORM_IOS
#		define SYS_PLATFORM_IOS_SIM
#		define SYS_ARCH_X86_64
#		define SYS_64BIT
#		define SYS_DEVICE_MOBILE
#	elif TARGET_OS_IPHONE == 1
#		define SYS_PLATFORM_IOS
#		define SYS_DEVICE_MOBILE
#		ifdef __LP64__
#			define SYS_ARCH_ARM64
#			define SYS_64BIT
#		else
#			define SYS_ARCH_ARM
#		endif
#	else
#		define SYS_PLATFORM_MAC
#		if defined(__ppc__)
#			ifdef __LP64__
#				define SYS_PLATFORM_MAC_PPC64
#				define SYS_ARCH_PPC64
#				define SYS_64BIT
#			else
#				define SYS_PLATFORM_MAC_PPC
#				define SYS_ARCH_PPC
#			endif
#		elif defined(__arm64__)
#			define SYS_PLATFORM_MAC_ARM64
#			define SYS_ARCH_ARM64
#			define SYS_64BIT
#		else
#			ifdef __LP64__
#				define SYS_PLATFORM_MAC_X86_64
#				define SYS_ARCH_X86_64
#				define SYS_64BIT
#			else
#				define SYS_PLATFORM_MAC_X86
#				define SYS_ARCH_X86
#			endif
#		endif
#	endif
#elif defined(__QNX__)
#	define SYS_PLATFORM_UNIX
#	define SYS_DEVICE_MOBILE
#	ifdef __arm__
#		define SYS_PLATFORM_BB10
#		ifdef __LP64__
#			define SYS_ARCH_ARM64
#			define SYS_64BIT
#		else
#			define SYS_ARCH_ARM
#		endif
#	else
#		define SYS_PLATFORM_BB10
#		ifdef __LP64__
#			define SYS_ARCH_X86_64
#			define SYS_64BIT
#		else
#			define SYS_ARCH_ARM
#		endif
#	endif
#elif defined(Macintosh)
#	define SYS_PLATFORM_MAC_CLASSIC
#	ifdef __ppc__
#		define SYS_PLATFORM_MAC_CLASSIC_PPC
#		define SYS_ARCH_PPC
#	else
#		define SYS_PLATFORM_MAC_CLASSIC_M68K
#		define SYS_ARCH_M68K
#	endif
#elif defined(__FreeBSD__)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_FREEBSD
#	define SYS_PLATFORM_X11
#	define SYS_PLATFORM_BSD
#	if defined(__arm__)
#		ifdef __LP64__
#			define SYS_PLATFORM_FREEBSD_ARM64
#			define SYS_ARCH_ARM64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_FREEBSD_ARM
#			define SYS_ARCH_ARM
#		endif
#	else
#		ifdef __LP64__
#			define SYS_PLATFORM_FREEBSD_X86_64
#			define SYS_ARCH_X86_64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_FREEBSD_X86
#			define SYS_ARCH_X86
#		endif
#	endif
#elif defined(__DragonFly__)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_DRAGONFLY
#	define SYS_PLATFORM_X11
#	define SYS_PLATFORM_BSD
#	if defined(__arm__)
#		ifdef __LP64__
#			define SYS_PLATFORM_DRAGONFLY_ARM64
#			define SYS_ARCH_ARM64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_DRAGONFLY_ARM
#			define SYS_ARCH_ARM
#		endif
#	else
#		ifdef __LP64__
#			define SYS_PLATFORM_DRAGONFLY_X86_64
#			define SYS_ARCH_X86_64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_DRAGONFLY_X86
#			define SYS_ARCH_X86
#		endif
#	endif
#elif defined(__NetBSD__)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_NETBSD
#	define SYS_PLATFORM_X11
#	define SYS_PLATFORM_BSD
#	if defined(__arm__)
#		ifdef __LP64__
#			define SYS_PLATFORM_NETBSD_ARM64
#			define SYS_ARCH_ARM64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_NETBSD_ARM
#			define SYS_ARCH_ARM
#		endif
#	else
#		ifdef __LP64__
#			define SYS_PLATFORM_NETBSD_X86_64
#			define SYS_ARCH_X86_64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_NETBSD_X86
#			define SYS_ARCH_X86
#		endif
#	endif
#elif defined(__OpenBSD__)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_OPENBSD
#	define SYS_PLATFORM_X11
#	define SYS_PLATFORM_BSD
#	if defined(__arm__)
#		ifdef __LP64__
#			define SYS_PLATFORM_OPENBSD_ARM64
#			define SYS_ARCH_ARM64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_OPENBSD_ARM
#			define SYS_ARCH_ARM
#		endif
#	else
#		ifdef __LP64__
#			define SYS_PLATFORM_OPENBSD_X86_64
#			define SYS_ARCH_X86_64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_OPENBSD_X86
#			define SYS_ARCH_X86
#		endif
#	endif
#elif defined(sun) || defined(__sun)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_SUNOS
#	define SYS_PLATFORM_X11
#	if defined(__sparc)
#		ifdef __LP64__
#			define SYS_PLATFORM_SUNOS_SPARC64
#			define SYS_ARCH_SPARC64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_SUNOS_SPARC
#			define SYS_ARCH_SPARC
#		endif
#	else
#		ifdef __LP64__
#			define SYS_PLATFORM_SUNOS_X86_64
#			define SYS_ARCH_X86_64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_SUNOS_X86
#			define SYS_ARCH_X86
#		endif
#	endif
#elif defined(sgi) || defined(__sgi)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_IRIX
#	define SYS_PLATFORM_X11
#	if defined(_MIPS_ISA_MIPS1) || defined(_MIPS_ISA_MIPS2) || defined(_MIPS_ISA_MIPS3)
#		define SYS_PLATFORM_IRIX_MIPS
#		define SYS_ARCH_MIPS
#	else
#		define SYS_PLATFORM_IRIX_MIPS64
#		define SYS_ARCH_MIPS64
#		define SYS_64BIT
#	endif
#elif defined(__BEOS__)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_BEOS
#	ifdef __ppc__
#		define SYS_PLATFORM_BEOS_PPC
#		define SYS_ARCH_PPC
#	else
#		define SYS_PLATFORM_BEOS_X86
#		define SYS_ARCH_X86
#	endif
#elif defined(__HAIKU__)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_BEOS
#	define SYS_PLATFORM_HAIKU
#	ifdef __LP64__
#		define SYS_PLATFORM_HAIKU_X86_64
#		define SYS_ARCH_X86_64
#		define SYS_64BIT
#	else
#		define SYS_PLATFORM_HAIKU_X86
#		define SYS_PLATFORM_BEOS_X86
#		define SYS_ARCH_X86
#	endif
#elif defined(_AIX)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_AIX
#	define SYS_PLATFORM_X11
#	ifdef __LP64__
#		define SYS_PLATFORM_AIX_PPC64
#		define SYS_ARCH_PPC64
#		define SYS_64BIT
#	else
#		define SYS_PLATFORM_AIX_PPC
#		define SYS_ARCH_PPC
#	endif
#elif defined(__hpux)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_HPUX
#	define SYS_PLATFORM_X11
#elif defined(__minix)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATFORM_MINIX
#	define SYS_PLATFORM_X11
#	if defined(__arm__)
#		define SYS_PLATFORM_MINIX_ARM
#		define SYS_ARCH_ARM
#	else
#		define SYS_PLATFORM_MINIX_X86
#		define SYS_ARCH_X86
#	endif
#elif defined(__gnu_hurd__)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATOFRM_HURD
#	define SYS_PLATFORM_HURD_X86
#	define SYS_PLATFORM_X11
#	define SYS_ARCH_X86
#elif defined(_SCO_DS)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATOFRM_OPENSERVER
#	define SYS_PLATFORM_OPENSERVER_X86
#	define SYS_PLATFORM_X11
#	define SYS_ARCH_X86
#elif defined(_UNIXWARE7)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATOFRM_UNIXWARE
#	define SYS_PLATFORM_UNIXWARE_X86
#	define SYS_PLATFORM_X11
#	define SYS_ARCH_X86
#elif defined(__osf__)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATOFRM_OSF
#	define SYS_PLATFORM_X11
#elif defined(__ultrix__)
#	define SYS_PLATFORM_UNIX
#	define SYS_PLATOFRM_ULTRIX
#	define SYS_PLATFORM_X11
#	if defined(vax)
#		define SYS_PLATFORM_ULTRIX_VAX
#		define SYS_ARCH_VAX
#	else
#		define SYS_PLATFORM_ULTRIX_MIPS
#		define SYS_ARCH_MIPS
#	endif
#elif defined(AMIGA)
#	define SYS_PLATFORM_AMIGAOS
#	ifdef __LP64__
#		define SYS_PLATFORM_AMIGAOS_PPC64
#		define SYS_ARCH_PPC64
#		define SYS_64BIT
#	else
#		define SYS_PLATFORM_AMIGAOS_PPC
#		define SYS_ARCH_PPC
#	endif
#elif defined(__MORPHOS__)
#	define SYS_PLATFORM_MORPHOS
#	ifdef __LP64__
#		define SYS_PLATFORM_MORPHOS_PPC64
#		define SYS_ARCH_PPC64
#		define SYS_64BIT
#	else
#		define SYS_PLATFORM_MORPHOS_PPC
#		define SYS_ARCH_PPC
#	endif
#elif defined(__ANDROID__)
#	define SYS_PLATFORM_ANDROID
#	define SYS_PLATFORM_UNIX
#	define SYS_DEVICE_MOBILE
#	if defined(__arm__)
#		ifdef __LP64__
#			define SYS_PLATFORM_ANDROID_ARM64
#			define SYS_ARCH_ARM64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_ANDROID_ARM
#			define SYS_ARCH_ARM
#		endif
#	elif defined(__ARM_ARCH)
#		define SYS_PLATFORM_ANDROID_ARM64
#		define SYS_ARCH_ARM64
#		define SYS_64BIT
#	elif defined(__mips__)
#		ifdef __LP64__
#			define SYS_PLATFORM_ANDROID_MIPS64
#			define SYS_ARCH_MIPS64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_ANDROID_MIPS
#			define SYS_ARCH_MIPS
#		endif
#	else
#		ifdef __LP64__
#			define SYS_PLATFORM_ANDROID_X86_64
#			define SYS_ARCH_X86_64
#			define SYS_64BIT
#		else
#			define SYS_PLATFORM_ANDROID_X86
#			define SYS_ARCH_X86
#		endif
#	endif
#elif defined(__DEVKITA64__)
#	define SYS_PLATFORM_NX
#	define SYS_ARCH_ARM64
#	define SYS_64BIT
#elif defined(GEKKO)
#	define SYS_PLATFORM_GAMECUBE
#	define SYS_PLATFORM_WII
#	define SYS_ARCH_PPC
#elif defined(__PPU__)
#	define SYS_PLATFORM_PS3
#	define SYS_PLATFORM_PS3_PPU
#	define SYS_ARCH_PPC64
#elif defined(__SPU__)
#	define SYS_PLATFORM_PS3
#	define SYS_PLATFORM_PS3_SPU
#	define SYS_ARCH_SPU
#elif defined(__MSDOS__)
#	define SYS_PLATFORM_MSDOS
#	define SYS_PLATFORM_MSDOS_X86
#	define SYS_ARCH_X86
#	define SYS_16BIT
#elif defined(__OS2__)
#	define SYS_PLATFORM_OS2
#	define SYS_PLATFORM_OS2_X86
#	define SYS_ARCH_X86
#elif defined(EPLAN9)
#	define SYS_PLATFORM_PLAN9
#elif defined(__VMS)
#	define SYS_PLATFORM_VMS
#elif defined(__VXWORKS__)
#	define SYS_PLATFORM_VXWORKS
#elif __STDC_HOSTED__ == 0
#	define SYS_PLATFORM_NONE
#endif

#endif /* _NE_SYSTEM_PLATFORM_DETECT_H_ */

/* NekoEngine
 *
 * PlatformDetect.h
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
