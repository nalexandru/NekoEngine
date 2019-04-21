#
# Misaki Build System Platform Detection
#

PLATFORM := $(shell uname -s)
PLATFORM_REL := $(shell uname -r)
PLATFORM_VER := $(shell uname -v)
PLATFORM_ARCH := $(shell uname -m)

HAVE_GCC := $(shell type -P gcc >/dev/null 2>&1 && echo true || echo false)
HAVE_CLANG := $(shell type -P clang >/dev/null 2>&1 && echo true || echo false)
HAVE_SUNCC := $(shell type -P suncc >/dev/null 2>&1 && echo true || echo false)
HAVE_AOCC := $(shell type -P aocc >/dev/null 2>&1 && echo true || echo false)
HAVE_ICC := $(shell type -P icc >/dev/null 2>&1 && echo true || echo false)

DLIB_PREFIX := lib

ifneq (,$(findstring mingw,$(CC)))
	PLATFORM := MinGW
endif

ifeq ($(PLATFORM), SunOS)
	ifeq ($(PLATFORM_REL), 5.10)
		include ../../mkconf/sunos_510_sunpro.mk
	else
		ifeq ($(CC),)
			ifeq ($(HAVE_SUNGCC), true)
				include ../../mkconf/sunos_511_sunpro.mk
			else
				include ../../mkconf/sunos_511_gcc.mk
			endif
		else
			ifeq ($(CC), suncc)
				include ../../mkconf/sunos_511_sunpro.mk
			else
				include ../../mkconf/sunos_511_gcc.mk
			endif
		endif
	endif
else ifeq ($(PLATFORM), OpenBSD)
	ifeq ($(CC),)
		ifeq ($(HAVE_GCC), true)
			include ../../mkconf/obsd_gcc.mk
		else
			include ../../mkconf/obsd_clang.mk
		endif
	else
		ifeq ($(CC), gcc)
			include ../../mkconf/obsd_gcc.mk
		else ifeq ($(CC), clang)
			include ../../mkconf/obsd_clang.mk
		endif
	endif
else ifeq ($(PLATFORM), FreeBSD)
	ifeq ($(CC),)
		ifeq ($(HAVE_GCC), true)
			include ../../mkconf/fbsd_gcc.mk
		else
			include ../../mkconf/fbsd_clang.mk
		endif
	else
		ifeq ($(CC), gcc)
			include ../../mkconf/fbsd_gcc.mk
		else ifeq ($(CC), clang)
			include ../../mkconf/fbsd_clang.mk
		endif
	endif
else ifeq ($(PLATFORM), NetBSD)
	include ../../mkconf/nbsd_gcc.mk
else ifeq ($(PLATFORM), OpenBSD)
	include ../../mkconf/obsd_gcc.mk
else ifeq ($(PLATFORM), Darwin)
	VER_MAJOR := $(shell uname -r | grep -E -o '[0-9]+?' - | cut -d$$'\n' -f1 -)
	ifeq ($(VER_MAJOR), 8)
		include ../../mkconf/darwin_8_gcc.mk
	else ifeq ($(VER_MAJOR), 9)
		include ../../mkconf/darwin_9_gcc.mk
	else ifeq ($(VER_MAJOR), 10)
		include ../../mkconf/darwin_9_gcc.mk
	else
		include ../../mkconf/darwin_clang.mk
	endif
else ifeq ($(PLATFORM), Linux)
	UNAME_A = $(shell uname -o | tr -d '\n')
	ifeq ($(UNAME_A), Android)
		PLATFORM := Android
		include ../../mkconf/android_gcc.mk
	else
		ifeq ($(CC),)
			ifeq ($(HAVE_SUNCC), true)
				include ../../mkconf/linux_sunpro.mk
			else ifeq ($(HAVE_GCC), true)
				include ../../mkconf/linux_gcc.mk
			else
				include ../../mkconf/linux_clang.mk
			endif
		else
			ifeq ($(CC), gcc)
				include ../../mkconf/linux_gcc.mk
			else ifeq ($(CC), clang)
				include ../../mkconf/linux_clang.mk
			else ifeq ($(CC), suncc)
				include ../../mkconf/linux_sunpro.mk
			endif
		endif
	endif
else ifeq ($(PLATFORM), MinGW)
	include ../../mkconf/mingw.mk
else
	include ../../mkconf/generic_gcc.mk
endif
