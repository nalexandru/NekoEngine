include ../../mkconf/platform.mk
include ../../mkconf/shared.mk

PROJ := kazmath
PROJ_CFLAGS := -I../include
PROJ_LDFLAGS :=
PROJ_BUILD_DIR := $(BUILD_DIR)/$(PROJ)
SRC := aabb2.c		\
	aabb3.c		\
	mat3.c		\
	mat4.c		\
	plane.c		\
	quaternion.c	\
	ray2.c		\
	ray3.c		\
	utility.c	\
	vec2.c		\
	vec3.c		\
	vec4.c
OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(SRC:%.c=%.o)))

vpath %.c $(dir $(SRC))

all: dir lib

dir:
	@mkdir -p $(BUILD_DIR)/$(PROJ); mkdir -p $(OUTPUT_DIR)

$(PROJ_BUILD_DIR)/%.o: %.c
	@echo "[CC] $<"; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PIC_CFLAGS) $(PLATFORM_CFLAGS) $< -o $@

lib: $(OBJ)
	@echo '[AR] $(PROJ)'; $(AR) rcs $(OUTPUT_DIR)/lib$(PROJ).a $(OBJ)

clean:
	@echo '[Clean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ)/*; rm -rf $(OUTPUT_DIR)/lib$(PROJ).a

distclean:
	@echo '[Distclean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ); rm -rf $(OUTPUT_DIR)/lib$(PROJ).a
