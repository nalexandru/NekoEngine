COMPAT_SRC :=
SYS_SRC :=

SYS_TYPE := Unknown

ifeq ($(PLATFORM), SunOS)
	SYS_TYPE := Unix

	ifeq ($(PLATFORM_REL), 5.10)
		COMPAT_SRC := src/system/compat/explicit_bzero.c	\
			src/system/compat/reallocarray.c		\
			src/system/compat/strlcat.c			\
			src/system/compat/strlcpy.c
	else
		COMPAT_SRC := src/system/compat/explicit_bzero.c	\
			src/system/compat/reallocarray.c		\
			src/system/compat/strlcat.c			\
			src/system/compat/strlcpy.c
	endif
else ifeq ($(PLATFORM), OpenBSD)
	SYS_TYPE := Unix
else ifeq ($(PLATFORM), FreeBSD)
	SYS_TYPE := Unix
else ifeq ($(PLATFORM), NetBSD)
	SYS_TYPE := Unix
else ifeq ($(PLATFORM), OpenBSD)
	SYS_TYPE := Unix
else ifeq ($(PLATFORM), Darwin)
	SYS_TYPE := Unix
	COMPAT_SRC := src/system/compat/explicit_bzero.c	\
		src/system/compat/reallocarray.c
else ifeq ($(PLATFORM), Linux)
	SYS_TYPE := Unix
	UNAME_A = $(shell uname -o | tr -d '\n')
	COMPAT_SRC := src/system/compat/explicit_bzero.c	\
		src/system/compat/reallocarray.c		\
		src/system/compat/strlcat.c			\
		src/system/compat/strlcpy.c
else ifeq ($(PLATFORM), MinGW)
	SYS_TYPE := Win32
	COMPAT_SRC := src/system/compat/explicit_bzero.c	\
		src/system/compat/reallocarray.c		\
		src/system/compat/strlcat.c			\
		src/system/compat/strlcpy.c
endif

ifeq ($(SYS_TYPE), Unix)
	SYS_SRC := src/system/unix/sys_unix.c			\
		src/system/unix/cond_var.c			\
		src/system/unix/thread.c			\
		src/system/unix/mutex.c				\
		src/system/unix/time.o
else ifeq ($(SYS_TYPE), Win32)
	SYS_SRC := src/system/win32/sys_win32.c			\
		src/system/win32/compat_win32.c			\
		src/system/win32/cond_var.c			\
		src/system/win32/thread.c			\
		src/system/win32/mutex.c			\
		src/system/win32/time.o
endif

COMPAT_OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(COMPAT_SRC:%.c=%.o)))
SYS_OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(SYS_SRC:%.c=%.o)))

vpath %.c $(dir $(COMPAT_SRC))
vpath %.c $(dir $(SYS_SRC))
