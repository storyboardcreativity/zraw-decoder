CC := gcc
CFL := -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11
TOOLCHAIN :=
OUT_ELF_NAME := zraw-decoder
ARCH :=
INCS := -Iinclude -Izraw-decoder-lib/include -Iquick_arg_parser -Itinydngloader
LIBS := -lcrypto -lpthread -lstdc++ -lm

BUILDDIR := build/

# Sources
SOURCE_FILES := src/main.cpp

all: check-and-reinit-submodules
	$(MAKE) -C ./zraw-decoder-lib
	@mkdir -p $(BUILDDIR)
	cp zraw-decoder-lib/build/libzraw.so $(BUILDDIR)libzraw.so
	$(TOOLCHAIN)$(CC) $(CFL) $(ARCH) $(SOURCE_FILES) $(INCS) $(LIBS) -L./$(BUILDDIR) -lzraw -o $(BUILDDIR)$(OUT_ELF_NAME)

check-and-reinit-submodules:
	@if git submodule status | egrep -q '^[-]|^[+]' ; then \
			echo "INFO: Need to reinitialize git submodules"; \
			git submodule update --init; \
	fi

clear:
	rm -rf $(BUILDDIR)
