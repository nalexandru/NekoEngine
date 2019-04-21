include mkconf/platform.mk
include mkconf/shared.mk

PROJ := miwa
PROJ_CFLAGS := -Iinclude
PROJ_LDFLAGS := -lssl -lcrypto
PROJ_BUILD_DIR := build/miwa
SHARED_SRC := src/runtime/array.c	\
	src/runtime/queue.c		\
	src/runtime/runtime.c		\
	src/runtime/string.c		\
	src/runtime/variant.c		\
	src/system/error.c		\
	src/system/log.c		\
	src/system/log_ssl.c		\
	src/system/system.c		\
	src/system/threadpool.c		\
	src/system/config.c
SHARED_OBJ := $(addprefix $(PROJ_BUILD_DIR)/,$(notdir $(SHARED_SRC:%.c=%.o)))

include proj.mk

vpath %.c $(dir $(SHARED_SRC))

all: dir conf lib

$(PROJ_BUILD_DIR)/%.o: %.c
	@echo '[CC] $<'; $(CC) $(CFLAGS) $(PIC_CFLAGS) $(PROJ_CFLAGS) $(PLATFORM_CFLAGS) $< -o $@

dir:
	@mkdir -p build/miwa/src/runtime; mkdir -p build/miwa/src/system/compat; mkdir -p build/miwa/src/system/unix; mkdir -p out;

conf:
	@tools/config.sh

lib: $(SHARED_OBJ) $(SYS_OBJ) $(COMPAT_OBJ)
	@echo '[AR] $(PROJ)'; $(AR) rcs out/lib$(PROJ).a $(COMPAT_OBJ) $(SYS_OBJ) $(SHARED_OBJ)

clean:
	@echo '[Clean] $(PROJ)'; rm -rf build/miwa/*; rm -rf bin/lib$(PROJ).a

distclean:
	@echo '[Distclean] $(PROJ)'; rm -rf build/miwa; rm -rf bin/lib$(PROJ).a
