include ../../mkconf/platform.mk
include ../../mkconf/shared.mk

PROJ := launcher
PROJ_CFLAGS := -I.
PROJ_LDFLAGS := -lNekoEngine -lX11 -lXm -lXt -lm
SRC := launcher_x11.c
OBJ := $(SRC:.c=.o)

ifeq ($(PLATFORM), Darwin)
	SRC := launcher_mac.m
	OBJ := $(SRC:.m=.o)
	PROJ_LDFLAGS := -lNekoEngine -framework Foundation -framework AppKit -framework Cocoa
else ifeq ($(PLATFORM), MinGW)
	SRC := launcher_win32.c
	OBJ := $(SRC:.c=.o)
	PROJ_LDFLAGS := -lNekoEngine -lgdi32 -lcomctl32
endif

.c.o:
	@echo '[CC] $<'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PLATFORM_CFLAGS) $< -o $(BUILD_DIR)/$(PROJ)/$@

.m.o:
	@echo '[CC] $<'; $(CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(PLATFORM_CFLAGS) $< -o $(BUILD_DIR)/$(PROJ)/$@

all: dir exe

dir:
	@mkdir -p $(BUILD_DIR)/$(PROJ); mkdir -p $(OUTPUT_DIR)

dbg:
	@echo $(SRC_ROOT)

exe: $(OBJ)
	@echo '[LINK] $(PROJ)'; $(CC) -o $(OUTPUT_DIR)/$(PROJ) $(addprefix $(BUILD_DIR)/$(PROJ)/, $(OBJ)) $(SHARED_LDFLAGS) $(PLATFORM_LDFLAGS) $(PROJ_LDFLAGS)

clean:
	@echo '[Clean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ)/*; rm -rf $(OUTPUT_DIR)/$(PROJ)

distclean:
	@echo '[Distclean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ); rm -rf $(OUTPUT_DIR)/$(PROJ).
