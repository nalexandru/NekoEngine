AR := ar
CC := suncc
PLATFORM_CFLAGS := -I/usr/local/include -xc99 -c -Wno-incompatible-pointer-types
PLATFORM_LDFLAGS := -L/usr/local/lib
PIC_CFLAGS := -Kpic
DLIB_CFLAGS := -shared
DLIB_EXT := so
WA_FLAGS := -Wl,--no-whole-archive

TOOLS_CC := $(CC)
TOOLS_CFLAGS := $(PLATFORM_CFLAGS)
TOOLS_LDFLAGS := $(PLATFORM_LDFLAGS)

ifndef CONFIG
	CONFIG = Debug
endif

ifeq ($(CONFIG), Release)
	PLATFORM_CFLAGS := $(PLATFORM_CFLAGS) -fma -xipo -xO -s
else ifeq ($(CONFIG), Debug)
	PLATFORM_CFLAGS := $(PLATFORM_CFLAGS) -g -g3 -xcheck -D_DEBUG
else
	$(error Unknown build configuration)
endif
