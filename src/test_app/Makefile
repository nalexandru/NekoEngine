include ../../mkconf/platform.mk
include ../../mkconf/shared.mk

PROJ := test_app
PROJ_CFLAGS := -I../include
PROJ_LDFLAGS := -lNekoEngine
PROJ_BUILD_DIR := $(BUILD_DIR)/$(PROJ)
SRC := test_app.c	\
	systems.c
ECS_SRC := $(BUILD_DIR)/$(PROJ)/ecs_test_app.ecs

OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(SRC:%.c=%.o)))

vpath %.c $(dir $(SRC))

all: dir ecsgen lib

dir:
	@mkdir -p $(BUILD_DIR)/$(PROJ); mkdir -p $(OUTPUT_DIR)

ecsgen:
	@echo '[ECS] $(CURDIR)'; $(OUTPUT_DIR)/ecsgen $(CURDIR) $(BUILD_DIR)/$(PROJ)/ecs_test_app.c;	\
	echo '[CC] ecs_test_app.c'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PIC_CFLAGS) $(PLATFORM_CFLAGS) $(BUILD_DIR)/$(PROJ)/ecs_test_app.c -o $(BUILD_DIR)/$(PROJ)/ecs_test_app.o

$(PROJ_BUILD_DIR)/%.o: %.c
	@echo '[CC] $<'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PIC_CFLAGS) $(PLATFORM_CFLAGS) $< -o $@

lib: $(OBJ) $(ECS_OBJ)
	@echo '[LINK] $(PROJ)'; $(CC) $(DLIB_CFLAGS) -o $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT) $(OBJ) $(BUILD_DIR)/$(PROJ)/ecs_test_app.o $(SHARED_LDFLAGS) $(PLATFORM_LDFLAGS) $(PROJ_LDFLAGS)

clean:
	@echo '[Clean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ)/*; rm -rf $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT)

distclean:
	@echo '[Distclean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ); rm -rf $(OUTPUT_DIR)/$(DLIB_PREFIX)$(PROJ).$(DLIB_EXT)
