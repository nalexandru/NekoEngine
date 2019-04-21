PLATFORM_CFLAGS := -std=c99 -c -Wno-incompatible-pointer-types
PLATFORM_LDFLAGS := -Wl,--export-all-symbols -static-libgcc
PIC_CFLAGS := -fPIC
DLIB_CFLAGS := -shared
DLIB_EXT := dll
DLIB_PREFIX := 
WA_FLAGS := -Wl,--no-whole-archive

TOOLS_CC := gcc
TOOLS_CFLAGS := $(PLATFORM_CFLAGS)
TOOLS_LDFLAGS :=

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
