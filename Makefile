BUILD_DIR := build
BUILD_TYPE ?= Release
CXX ?= g++

.PHONY: test configure build clean

test: configure build
	@cmake --build "$(BUILD_DIR)" --target test --config "$(BUILD_TYPE)" -- -j

configure:
	@cmake -S . -B "$(BUILD_DIR)" -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" -DCMAKE_CXX_COMPILER="$(CXX)"

build:
	@cmake --build "$(BUILD_DIR)" --config "$(BUILD_TYPE)" -- -j

clean:
	@cmake --build "$(BUILD_DIR)" --target clean --config "$(BUILD_TYPE)" || true
	@rm -rf "$(BUILD_DIR)"
