include ../../mkconf/platform.mk
include ../../mkconf/shared.mk

PROJ := negfx_gl
PROJ_CFLAGS := -I../include
PROJ_LDFLAGS := -lGL
PROJ_BUILD_DIR := $(BUILD_DIR)/$(PROJ)
SRC := buffer.c		\
	debug.c		\
	glad.c		\
	gl_gfx.c	\
	gl_gfx_unix.c	\
	shader.c	\
	texture.c	\
	systems.c
OBJ := $(SRC:.c=.o)

vpath %.c $(dir $(SRC))

all: dir lib

dir:
	@mkdir -p $(BUILD_DIR)/$(PROJ); mkdir -p $(OUTPUT_DIR)

$(PROJ_BUILD_DIR)/%.o: %.c
	@echo '[CC] $<'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PIC_CFLAGS) $(PLATFORM_CFLAGS) $< -o $@

lib: $(OBJ)
	@echo '[LINK] $(PROJ)'; $(CC) $(DLIB_CFLAGS) -o $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT) $(OBJ) $(SHARED_LDFLAGS) $(PLATFORM_LDFLAGS) $(PROJ_LDFLAGS)

clean:
	@echo '[Clean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ)/*; rm -rf $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT)

distclean:
	@echo '[Distclean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ); rm -rf $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT)
