include ../../mkconf/platform.mk
include ../../mkconf/shared.mk

PROJ := NekoEngine
PROJ_CFLAGS := -D_NE_ENGINE_INTERNAL_ -I../include
PROJ_LDFLAGS := -lvorbisfile -lvorbis -logg -lphysfs -L../../deps/miwa/out $(WA_FLAGS) -lmiwa -lkazmath -lfreetype
PROJ_BUILD_DIR := $(BUILD_DIR)/$(PROJ)
SRC := engine.c				\
	io.c				\
	task.c				\
	json.c				\
	input.c				\
	resource.c			\
	application.c			\
	graphics/graphics.c		\
	graphics/texture.c		\
	graphics/material.c		\
	graphics/mesh.c			\
	graphics/nullgfx.c		\
	graphics/primitive.c		\
	sound/sound.c			\
	sound/sound_clip.c		\
	sound/sound_src.c		\
	scene/bounds.c			\
	scene/camera.c			\
	scene/frustum.c			\
	scene/octree.c			\
	scene/scene.c			\
	ecs/ecsys.c			\
	ecs/entity.c			\
	ecs/component.c			\
	gui/gui.c			\
	gui/font.c			\
	gui/widget.c			\
	asset/dds.c			\
	asset/png.c			\
	asset/tga.c			\
	asset/nmesh.c			\
	asset/meta.c
OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(SRC:%.c=%.o)))

include proj.mk

vpath %.c $(dir $(SRC))
vpath %.c $(dir $(SYS_SRC))

PROJ_LDFLAGS := $(OPENAL_LIB) $(PROJ_LDFLAGS)

all: dir ecsgen resources lib

dir:
	@mkdir -p $(PROJ_BUILD_DIR)/graphics; mkdir -p $(PROJ_BUILD_DIR)/sound; mkdir -p $(PROJ_BUILD_DIR)/scene; mkdir -p $(PROJ_BUILD_DIR)/mac; mkdir -p $(PROJ_BUILD_DIR)/unix; mkdir -p $(PROJ_BUILD_DIR)/gui; mkdir -p $(PROJ_BUILD_DIR)/asset; mkdir -p $(OUTPUT_DIR)

resources:
	@echo '[ZIP] $(SRC_ROOT)/src/engine/res'; cd $(SRC_ROOT)/src/engine/res; zip -r $(PROJ_BUILD_DIR)/engine_res.zip *;	\
	echo '[BIN2C] engine_res.zip'; $(OUTPUT_DIR)/bin2c -n engine_res -c $(PROJ_BUILD_DIR)/engine_res.zip $(PROJ_BUILD_DIR)/engine_res.c;	\
	echo '[CC] engine_res.c'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PIC_CFLAGS) $(PLATFORM_CFLAGS) $(PROJ_BUILD_DIR)/engine_res.c -o $(PROJ_BUILD_DIR)/engine_res.o

ecsgen:
	@echo '[ECS] $(SRC_ROOT)/include'; $(OUTPUT_DIR)/ecsgen $(SRC_ROOT)/include $(PROJ_BUILD_DIR)/ecs_engine.c;	\
	echo '[CC] ecs_engine.c'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PIC_CFLAGS) $(PLATFORM_CFLAGS) $(PROJ_BUILD_DIR)/ecs_engine.c -o $(PROJ_BUILD_DIR)/ecs_engine.o

$(PROJ_BUILD_DIR)/%.o: %.c
	@echo '[CC] $<'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PIC_CFLAGS) $(PLATFORM_CFLAGS) $< -o $@

$(PROJ_BUILD_DIR)/%.o: %.m
	@echo '[CC] $<'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PIC_CFLAGS) $(PLATFORM_CFLAGS) $< -o $@

lib: $(OBJ) $(SYS_OBJ)
	@echo '[LINK] $(PROJ)'; $(CC) $(DLIB_CFLAGS) -o $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT) $(OBJ) $(SYS_OBJ) $(BUILD_DIR)/$(PROJ)/ecs_engine.o $(BUILD_DIR)/$(PROJ)/engine_res.c $(SHARED_LDFLAGS) $(PLATFORM_LDFLAGS) $(PROJ_LDFLAGS)

clean:
	@echo '[Clean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ)/*; rm -rf $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT)

distclean:
	@echo '[Distclean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ); rm -rf $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT)
