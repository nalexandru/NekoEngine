IOS_SDK := $(shell xcrun --sdk iphoneos --show-sdk-path)
AR := ar
CC := clang -isysroot $(IOS_SDK) -arch arm64
PLATFORM_CFLAGS := -I/opt/ios/local/include -I/opt/ios/local/opt/openssl/include -std=c99 -c -Wno-switch
PLATFORM_LDFLAGS := -L/opt/ios/local/lib -L/opt/ios/local/opt/openssl/lib -bind_at_load -Wl,-rpath .
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
