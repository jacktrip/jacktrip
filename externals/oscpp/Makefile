.PHONY: build clean distclean test v verbose

NINJA_FLAGS = $(args)

ifeq (1,$(or $(verbose),$(v)))
NINJA_FLAGS += -v
endif

build: build/CMakeCache.txt
	cd build && ninja $(NINJA_FLAGS)

v: verbose

verbose: NINJA_FLAGS += -v
verbose: build

build/CMakeCache.txt:
	mkdir -p build
	cd build && cmake -G Ninja ..

clean:
	cd build && ninja clean

distclean:
	rm -rf build

test: build
	cd build && ctest -V
