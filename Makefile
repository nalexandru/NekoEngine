#
# NekoEngine Makefile
#

.PHONY: all clean tools doc
MAKE := make
CFLAGS := $(CFLAGS)
SRC_ROOT := $(shell pwd)
PROJ_DIR := deps/miwa src/kazmath tools/ecsgen src/engine src/negfx_vk src/negfx_gl1 src/negfx_gl src/test_app src/launcher

include mkconf/shared.mk

ifeq ($(shell type -P gmake >/dev/null 2>&1 && echo true || echo false), true)
	MAKE := gmake
endif

all: dir miwa kazmath tools engine negfx_vk test_app launcher

tools: ecsgen bin2c

miwa:
	@echo 'Building miwa'; cd deps/miwa; $(MAKE) -s; cd $(SRC_ROOT);

kazmath:
	@echo 'Building kazmath'; cd src/kazmath; $(MAKE) -s; cd $(SRC_ROOT);

engine: miwa kazmath ecsgen
	@echo 'Building engine'; cd src/engine; $(MAKE) -s; cd $(SRC_ROOT);

launcher: engine
	@echo 'Building launcher'; cd src/launcher; $(MAKE) -s; cd $(SRC_ROOT);

negfx_gl: engine
	@echo 'Building negfx_gl'; cd src/negfx_gl; $(MAKE) -s; cd $(SRC_ROOT);

negfx_gl1: engine
	@echo 'Building negfx_gl1'; cd src/negfx_gl1; $(MAKE) -s; cd $(SRC_ROOT);

negfx_gles2: engine
	@echo 'Building negfx_gles2'; cd src/negfx_gles2; $(MAKE) -s; cd $(SRC_ROOT);

negfx_vk: engine
	@echo 'Building negfx_vk'; cd src/negfx_vk; $(MAKE) -s; cd $(SRC_ROOT);

test_app: engine
	@echo 'Building test_app'; cd src/test_app; $(MAKE) -s; cd $(SRC_ROOT);

ecsgen:
	@echo 'Building ecsgen'; cd tools/ecsgen; $(MAKE) -s; cd $(SRC_ROOT);

bin2c:
	@echo 'Building bin2c'; cd tools/bin2c; $(MAKE) -s; cd $(SRC_ROOT);

doc:
	doxygen;

clean:
	@for d in $(PROJ_DIR);			\
	do					\
	cd $$d; $(MAKE) -s clean; cd ../../;	\
	done;

distclean:
	@for d in $(PROJ_DIR);				\
	do						\
	cd $$d; $(MAKE) -s distclean; cd ../../;		\
	done; rm -rf $(OUTPUT_DIR); rm -rf $(BUILD_DIR);

dir:
	@mkdir -p $(OUTPUT_DIR); mkdir -p $(BUILD_DIR);

link:
	@echo 'Linking data directory'; cd bin; ln -s ../data data; cd $(SRC_ROOT);
