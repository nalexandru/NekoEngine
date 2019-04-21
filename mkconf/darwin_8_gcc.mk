AR := ar
CC := gcc
PLATFORM_CFLAGS := -DOSX -I/usr/local/include -I/usr/local/opt/openssl/include -std=c99 -c
PLATFORM_LDFLAGS := -L/usr/local/lib -L/usr/local/opt/openssl/lib -framework Foundation -framework IOKit 
PIC_CFLAGS := -fPIC
DLIB_CFLAGS := -dynamiclib
DLIB_EXT := dylib
WA_FLAGS := -Wl,-all_load

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
