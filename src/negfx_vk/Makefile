include ../../mkconf/platform.mk
include ../../mkconf/shared.mk

PROJ := negfx_vk
PROJ_CFLAGS := -I../include
PROJ_LDFLAGS := -lNekoEngine -lvulkan
PROJ_BUILD_DIR := $(BUILD_DIR)/$(PROJ)
SRC := vkgfx.c		\
	debug.c		\
	swapchain.c	\
	init.c		\
	buffer.c	\
	gui.c		\
	shader.c	\
	pipeline.c	\
	renderpass.c	\
	render.c		\
	render_classic.c	\
	render_rt.c		\
	dpool.c		\
	texture.c
WSI_SRC := vkgfx_unix.c
OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(SRC:%.c=%.o)))
WSI_OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(WSI_SRC:%.c=%.o)))
SHADER_OUTPUT_DIR := $(OUTPUT_DIR)/data/shaders

VS_SRC := shaders/depth_vert.vert \
	shaders/gui_vert.vert \
	shaders/lighting_vert.vert
VS_OBJ := $(addprefix $(SHADER_OUTPUT_DIR)/,$(notdir $(VS_SRC:%.vert=%.spv)))

FS_SRC := shaders/depth_frag.frag \
	shaders/gui_frag.frag \
	shaders/lighting_frag.frag
FS_OBJ := $(addprefix $(SHADER_OUTPUT_DIR)/,$(notdir $(FS_SRC:%.frag=%.spv)))

CS_SRC := shaders/light_culling.comp
CS_OBJ := $(addprefix $(SHADER_OUTPUT_DIR)/,$(notdir $(CS_SRC:%.comp=%.spv)))

ifeq ($(PLATFORM), Darwin)
	PROJ_CFLAGS := $(PROJ_CFLAGS) -I/usr/local/MoltenVK/include
	PROJ_LDFLAGS := -lNekoEngine -L/usr/local/MoltenVK/macOS/dynamic -lMoltenVK
	WSI_SRC := vkgfx_mac.m
	WSI_OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(WSI_SRC:%.m=%.o)))
	vpath %.m $(dir $(WSI_SRC))
endif

vpath %.c $(dir $(SRC))
vpath %.vert $(dir $(VS_SRC))
vpath %.frag $(dir $(FS_SRC))
vpath %.comp $(dir $(CS_SRC))

all: dir lib shaders

dir:
	@mkdir -p $(BUILD_DIR)/$(PROJ); mkdir -p $(OUTPUT_DIR)

resources:
	@echo '[ZIP] $(PROJ_BUILD_DIR)'; cd $(SRC_ROOT)/src/engine/res; zip -r $(PROJ_BUILD_DIR)/engine_res.zip *;	\
	echo '[BIN2C] engine_res.zip'; $(OUTPUT_DIR)/bin2c -n engine_res -c $(PROJ_BUILD_DIR)/engine_res.zip $(PROJ_BUILD_DIR)/engine_res.c;	\
	echo '[CC] engine_res.c'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PIC_CFLAGS) $(PLATFORM_CFLAGS) $(PROJ_BUILD_DIR)/engine_res.c -o $(PROJ_BUILD_DIR)/engine_res.o

shaders: $(VS_OBJ) $(FS_OBJ) $(CS_OBJ)

$(SHADER_OUTPUT_DIR)/%.spv: %.vert
	@echo '[GLSLC] $<'; glslc --target-env=vulkan -I$(SRC_ROOT)/src/negfx_vk/shaders/include $< -o $@

$(SHADER_OUTPUT_DIR)/%.spv: %.frag
	@echo '[GLSLC] $<'; glslc --target-env=vulkan -I$(SRC_ROOT)/src/negfx_vk/shaders/include $< -o $@

$(SHADER_OUTPUT_DIR)/%.spv: %.comp
	@echo '[GLSLC] $<'; glslc --target-env=vulkan -I$(SRC_ROOT)/src/negfx_vk/shaders/include $< -o $@

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
