AR := ar
CC := gcc
PLATFORM_CFLAGS := -I/usr/local/include -std=c99
PLATFORM_LDFLAGS := -L/usr/local/lib
PIC_CFLAGS := -fPIC
DLIB_CFLAGS := -shared
DLIB_EXT := so
WA_FLAGS := -Wl,--whole-archive

TOOLS_CC := $(CC)
TOOLS_CFLAGS := $(PLATFORM_CFLAGS)
TOOLS_LDFLAGS := $(PLATFORM_LDFLAGS)

ifndef CONFIG
	CONFIG = Debug
endif

ifeq ($(CONFIG), Release)
	PLATFORM_CFLAGS := $(PLATFORM_CFLAGS) -O3 -fomit-frame-pointer
else ifeq ($(CONFIG), Debug)
	PLATFORM_CFLAGS := $(PLATFORM_CFLAGS) -D_DEBUG -g2
else
	$(error Unknown build configuration)
endif
