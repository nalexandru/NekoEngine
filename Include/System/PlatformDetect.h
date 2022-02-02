#ifndef _NE_SYSTEM_PLATFORM_DETECT_H_
#define _NE_SYSTEM_PLATFORM_DETECT_H_

#if defined(_WIN64)
	#define SYS_PLATFORM_WIN64
	#define SYS_PLATFORM_WINDOWS
	#define SYS_ARCH_X86_64
	#define SYS_64BIT
	#if defined(__MINGW64__)
		#define SYS_PLATFORM_MINGW
	#endif
#elif defined(_XBOX)
	#ifdef _XENON
		#define SYS_PLATFORM_XBOX_360
		#define SYS_ARCH_PPC64
	#else
		#define SYS_PLATFORM_XBOX
		#define SYS_ARCH_X86
	#endif
#elif defined(_WIN32)
	#define SYS_PLATFORM_WIN32
	#define SYS_PLATFORM_WINDOWS
	#define SYS_ARCH_X86
	#if defined(__MINGW32__)
		#define SYS_PLATFORM_MINGW
	#endif
#elif defined(__linux__) && !defined(__ANDROID__)
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
	#elif defined(__ARM_ARCH)
		#define SYS_PLATFORM_LINUX_ARM64
		#define SYS_ARCH_ARM64
		#define SYS_64BIT
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
	#elif defined(__hppa__)
		#define SYS_PLATFORM_LINUX_HPPA
		#define SYS_ARCH_HPPA
	#elif defined(__alpha__)
		#define SYS_PLATFORM_LINUX_ALPHA
		#define SYS_ARCH_ALPHA
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
		#if defined(__ppc__)
			#ifdef __LP64__
				#define SYS_PLATFORM_MAC_PPC64
				#define SYS_ARCH_PPC64
				#define SYS_64BIT
			#else
				#define SYS_PLATFORM_MAC_PPC
				#define SYS_ARCH_PPC
			#endif
		#else
			#ifdef __LP64__
				#define SYS_PLATFORM_MAC_X86_64
				#define SYS_ARCH_X86_64
				#define SYS_64BIT
			#else
				#define SYS_PLATFORM_MAC_X86
				#define SYS_ARCH_X86
			#endif
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
#elif defined(__MACOS_CLASSIC__)

#elif defined(__FreeBSD__)
	#define SYS_PLATFORM_UNIX
	#define SYS_PLATFORM_FREEBSD
	#define SYS_PLATFORM_X11
	#define SYS_PLATFORM_BSD
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
	#define SYS_PLATFORM_BSD
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
	#define SYS_PLATFORM_BSD
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
	#define SYS_PLATFORM_BSD
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
#elif defined(sgi) || defined(__sgi)
	#define SYS_PLATFORM_UNIX
	#define SYS_PLATFORM_IRIX
	#define SYS_PLATFORM_X11
	#if defined(_MIPS_ISA_MIPS1) || defined(_MIPS_ISA_MIPS2) || \
		defined(_MIPS_ISA_MIPS3)
		#define SYS_PLATFORM_IRIX_MIPS
		#define SYS_ARCH_MIPS
	#else
		#define SYS_PLATFORM_IRIX_MIPS64
		#define SYS_ARCH_MIPS64
		#define SYS_64BIT
	#endif
#elif defined(__BEOS__) || defined(__HAIKU__)
	#define SYS_PLATFORM_UNIX
	#define SYS_PLATFORM_BEOS
#elif defined(__ANDROID__)
	#define SYS_PLATFORM_ANDROID
	#define SYS_PLATFORM_UNIX
#elif defined(__DEVKITA64__)
	#define SYS_PLATFORM_NX
	#define SYS_ARCH_ARM64
	#define SYS_64BIT
#elif defined(GEKKO)
	#define SYS_PLATFORM_GAMECUBE
	#define SYS_PLATFORM_WII
	#define SYS_ARCH_PPC
#elif defined(__PPU__)
	#define SYS_PLATFORM_PS3
	#define SYS_PLATFORM_PS3_PPU
	#define SYS_ARCH_PPC64
#elif defined(__SPU__)
	#define SYS_PLATFORM_PS3
	#define SYS_PLATFORM_PS3_SPU
	#define SYS_ARCH_PPC
#endif

#endif /* _NE_SYSTEM_PLATFORM_DETECT_H_ */

