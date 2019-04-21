SYS_SRC :=
OPENAL_LIB := -lopenal
WS_TYPE := Unknown
FT_CFLAGS := $(shell pkg-config --cflags freetype2) -I/usr/local/include/freetype2
PLATFORM_CFLAGS := $(PLATFORM_CFLAGS) $(FT_CFLAGS)

ifeq ($(PLATFORM), SunOS)
	WS_TYPE := X11
else ifeq ($(PLATFORM), OpenBSD)
	WS_TYPE := X11
else ifeq ($(PLATFORM), FreeBSD)
	WS_TYPE := X11
else ifeq ($(PLATFORM), NetBSD)
	WS_TYPE := X11
else ifeq ($(PLATFORM), OpenBSD)
	WS_TYPE := X11
else ifeq ($(PLATFORM), Darwin)
	SYS_SRC := mac/engine_mac.m				\
		mac/input_mac.m						\
		mac/window_mac.m					\
		mac/mac_classes.m
	SYS_OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(SYS_SRC:%.m=%.o)))
	vpath %.m $(dir $(SYS_SRC))
	OPENAL_LIB := -framework OpenAL
	PLATFORM_LDFLAGS := $(PLATFORM_LDFLAGS) -framework Foundation -framework AppKit -framework Cocoa -framework Carbon
	VER_MAJOR := $(shell uname -r | grep -E -o '[0-9]+?' - | cut -d$$'\n' -f1 -)
	ifeq ($(VER_MAJOR), 8)
	else ifeq ($(VER_MAJOR), 9)
	else ifeq ($(VER_MAJOR), 10)
	else
		PLATFORM_LDFLAGS := $(PLATFORM_LDFLAGS) -framework QuartzCore
	endif
else ifeq ($(PLATFORM), Linux)
	WS_TYPE := X11
	UNAME_A = $(shell uname -o | tr -d '\n')
	PROJ_LDFLAGS := $(PROJ_LDFLAGS) -usys_mutex_lock -urt_queue_init
else ifeq ($(PLATFORM), MinGW)
	PLATFORM_CFLAGS := $(PLATFORM_CFLAGS) -I../../3rdparty/win/include
	PLATFORM_LDFLAGS := $(PLATFORM_LDFLAGS) -L../../3rdparty/win/lib/mingw -lgdi32
	OPENAL_LIB := -lOpenAL32
	WS_TYPE := Win32
	SYS_SRC := win32/engine_win32.c 	\
		win32/input_win32.c		\
		win32/window_win32.c
	SYS_OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(SYS_SRC:%.c=%.o)))
endif

ifeq ($(WS_TYPE), X11)
	SYS_SRC := unix/engine_unix.c			\
		unix/input_unix.c			\
		unix/window_unix.c
	SYS_OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(SYS_SRC:%.c=%.o)))
endif

ifeq ($(CONFIG), Release)
	PLATFORM_CFLAGS := $(PLATFORM_CFLAGS) -DNE_CONFIG_DEBUG
else ifeq ($(CONFIG), Deveopment)
	PLATFORM_CFLAGS := $(PLATFORM_CFLAGS) -DNE_CONFIG_DEVELOPMENT
else
	PLATFORM_CFLAGS := $(PLATFORM_CFLAGS) -DNE_CONFIG_RELEASE
endif
