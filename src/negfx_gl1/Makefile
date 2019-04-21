include ../../mkconf/platform.mk
include ../../mkconf/shared.mk

PROJ := negfx_gl1
PROJ_CFLAGS := -I../include -Iinclude
PROJ_LDFLAGS := -lNekoEngine -lGL
PROJ_BUILD_DIR := $(BUILD_DIR)/$(PROJ)
SRC := gl1gfx.c	\
	texture.c	\
	gui.c		\
	buffer.c	\
	render.c
WSI_SRC := gl1gfx_unix.c
OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(SRC:%.c=%.o)))
WSI_OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(WSI_SRC:%.c=%.o)))

ifeq ($(PLATFORM), Darwin)
	PROJ_CFLAGS := $(PROJ_CFLAGS)
	PROJ_LDFLAGS := -lNekoEngine -framework Cocoa -framework OpenGL
	WSI_SRC := gl1gfx_mac.m
	WSI_OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(WSI_SRC:%.m=%.o)))
	vpath %.m $(dir $(WSI_SRC))
else ifeq ($(PLATFORM), MinGW)
	PROJ_LDFLAGS := -lNekoEngine -lopengl32 -lgdi32
	WSI_SRC := gl1gfx_win32.c
	WSI_OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(WSI_SRC:%.c=%.o)))
endif

vpath %.c $(dir $(SRC))

all: dir lib

dir:
	@mkdir -p $(BUILD_DIR)/$(PROJ); mkdir -p $(OUTPUT_DIR)

$(PROJ_BUILD_DIR)/%.o: %.c
	@echo '[CC] $<'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PIC_CFLAGS) $(PLATFORM_CFLAGS) $< -o $@

$(PROJ_BUILD_DIR)/%.o: %.m
	@echo '[CC] $<'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PIC_CFLAGS) $(PLATFORM_CFLAGS) $< -o $@

lib: $(OBJ) $(WSI_OBJ)
	@echo '[LINK] $(PROJ)'; $(CC) $(DLIB_CFLAGS) -o $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT) $(OBJ) $(WSI_OBJ) $(SHARED_LDFLAGS) $(PLATFORM_LDFLAGS) $(PROJ_LDFLAGS)

clean:
	@echo '[Clean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ)/*; rm -rf $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT)

distclean:
	@echo '[Distclean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ); rm -rf $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT)
