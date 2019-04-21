AR := gar
CC := suncc
PLATFORM_CFLAGS := -DSUNOS -I/usr/local/include -I/opt/csw/include -I/opt/csw/include/libxml2 -xc99 -c
PLATFORM_LDFLAGS := -L/usr/local/lib -L/opt/csw/lib -lrt -lm
PIC_CFLAGS := -Kpic
DLIB_CFLAGS := -shared
DLIB_EXT := so
WA_FLAGS := -Wl,--whole-archive

TOOLS_CC := $(CC)
TOOLS_CFLAGS := $(PLATFORM_CFLAGS)
TOOLS_LDFLAGS := $(PLATFORM_LDFLAGS)

ifneq ($(PLATFORM_ARCH), i86pc)
	PIC_CFLAGS := -xcode=pic32
endif

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
