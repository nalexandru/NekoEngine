include ../../mkconf/platform.mk
include ../../mkconf/shared.mk

PROJ := bin2c
PROJ_CFLAGS := 
PROJ_LDFLAGS := 
SRC := bin2c.c
OBJ := $(SRC:.c=.o)

.c.o:
	@echo '[CC] $<'; $(TOOLS_CC) $(CFLAGS) $(PROJ_CFLAGS) $(SHARED_CFLAGS) $(TOOLS_CFLAGS) $< -o $(BUILD_DIR)/$(PROJ)/$@

all: dir exe

dir:
	@mkdir -p $(BUILD_DIR)/$(PROJ); mkdir -p $(OUTPUT_DIR)

exe: $(OBJ)
	@echo '[LINK] $(PROJ)'; $(TOOLS_CC) -o $(OUTPUT_DIR)/$(PROJ) $(addprefix $(BUILD_DIR)/$(PROJ)/, $(OBJ)) $(SHARED_LDFLAGS) $(TOOLS_LDFLAGS) $(PROJ_LDFLAGS)

clean:
	@echo '[Clean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ)/*; rm -rf $(OUTPUT_DIR)/$(PROJ)

distclean:
	@echo '[Distclean] $(PROJ)'; rm -rf $(BUILD_DIR)/$(PROJ); rm -rf $(OUTPUT_DIR)/$(PROJ).
