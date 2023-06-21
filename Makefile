# Get OS name
UNAME := $(shell uname)

CC := gcc
CFL := -D_FILE_OFFSET_BITS=64 -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11
ifeq ($(UNAME),Darwin)
MACOS_VERSION := 11.0# 10.15
CFL += -mmacosx-version-min=$(MACOS_VERSION)
endif

TOOLCHAIN :=
OUT_ELF_NAME := zraw-decoder
ARCH :=

INCS := -Iinclude -Izraw-decoder-lib/include -Iquick_arg_parser -Itinydngloader
LIBS := -lcrypto -lpthread -lstdc++ -lm

ifeq ($(UNAME),Darwin)
INCS_X86_64 := $(INCS)
INCS_ARM64 :=  $(INCS)
LIBS_X86_64 := -lpthread -lstdc++ -lm
LIBS_ARM64 :=  -lpthread -lstdc++ -lm
endif

STATIC_LIBS := zraw-decoder-lib/build/libzraw.a

BUILDDIR := build/

# Sources
SOURCE_FILES := src/main.cpp

all: check-and-reinit-submodules
	$(MAKE) -C ./zraw-decoder-lib
	@mkdir -p $(BUILDDIR)
ifeq ($(UNAME),Darwin)
	$(TOOLCHAIN)$(CC) $(CFL) -arch x86_64 $(SOURCE_FILES) $(INCS) $(STATIC_LIBS) $(LIBS_X86_64) -L./$(BUILDDIR) -o $(BUILDDIR)x86_64_$(OUT_ELF_NAME)
	$(TOOLCHAIN)$(CC) $(CFL) -arch arm64 $(SOURCE_FILES) $(INCS) $(STATIC_LIBS) $(LIBS_ARM64) -L./$(BUILDDIR) -o $(BUILDDIR)arm64_$(OUT_ELF_NAME)
	lipo $(BUILDDIR)x86_64_$(OUT_ELF_NAME) $(BUILDDIR)arm64_$(OUT_ELF_NAME) -create -output $(BUILDDIR)$(OUT_ELF_NAME)
else
	$(TOOLCHAIN)$(CC) $(CFL) $(ARCH) $(SOURCE_FILES) $(INCS) $(STATIC_LIBS) $(LIBS) -L./$(BUILDDIR) -o $(BUILDDIR)$(OUT_ELF_NAME)
endif

check-and-reinit-submodules:
	@if git submodule status | egrep -q '^[-]|^[+]' ; then \
			echo "INFO: Need to reinitialize git submodules"; \
			git submodule update --init; \
	fi

clear:
	rm -rf $(BUILDDIR)
