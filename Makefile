CC := gcc
CFL := -D_FILE_OFFSET_BITS=64 -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11
TOOLCHAIN :=
OUT_ELF_NAME := zraw-decoder
ARCH :=
INCS := -Iinclude -Izraw-decoder-lib/include -Iquick_arg_parser -Itinydngloader
LIBS := -lcrypto -lpthread -lstdc++ -lm
STATIC_LIBS := zraw-decoder-lib/build/libzraw.a

BUILDDIR := build/

# Sources
SOURCE_FILES := src/main.cpp

all: check-and-reinit-submodules
	$(MAKE) -C ./zraw-decoder-lib
	@mkdir -p $(BUILDDIR)
	$(TOOLCHAIN)$(CC) $(CFL) $(ARCH) $(SOURCE_FILES) $(INCS) $(STATIC_LIBS) $(LIBS) -L./$(BUILDDIR) -o $(BUILDDIR)$(OUT_ELF_NAME)

check-and-reinit-submodules:
	@if git submodule status | egrep -q '^[-]|^[+]' ; then \
			echo "INFO: Need to reinitialize git submodules"; \
			git submodule update --init; \
	fi

clear:
	rm -rf $(BUILDDIR)
