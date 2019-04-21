#
# Misaki Build System Shared Variables
#

SRC_ROOT := $(CURDIR)/../..
OUTPUT_DIR := $(SRC_ROOT)/bin
BUILD_DIR := $(SRC_ROOT)/build
SHARED_CFLAGS := -D_REENTRANT -D__EXTENSIONS__ -Iinclude -I../../include -I../../deps/miwa/include
SHARED_LDFLAGS := -L$(OUTPUT_DIR)
