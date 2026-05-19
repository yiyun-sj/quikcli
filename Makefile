BUILD_DIR := build

.PHONY: all configure configure-tests configure-examples configure-all build test clean distclean

all: test

configure:
	cmake -B $(BUILD_DIR) -S .

configure-tests:
	cmake -B $(BUILD_DIR) -S . -DQUIKCLI_BUILD_TESTS=ON

configure-examples:
	cmake -B $(BUILD_DIR) -S . -DQUIKCLI_BUILD_EXAMPLES=ON

configure-all:
	cmake -B $(BUILD_DIR) -S . -DQUIKCLI_BUILD_TESTS=ON -DQUIKCLI_BUILD_EXAMPLES=ON

build:
	cmake --build $(BUILD_DIR)

test: configure-tests build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

examples: configure-examples build

all: configure-all build

clean:
	cmake --build $(BUILD_DIR) --target clean

distclean:
	rm -rf $(BUILD_DIR)