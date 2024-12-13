#!/bin/bash -ex

cd "$(dirname "$0")"

mkdir -p build-emscripten-release && cd build-emscripten-release

mkdir -p ~/www

cmake -DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_TOOLCHAIN_FILE=toolchains/generic/Emscripten-wasm.cmake \
	-DCMAKE_PREFIX_PATH=~/deps -DCMAKE_FIND_ROOT_PATH=~/deps -DCMAKE_INSTALL_PREFIX=~/www/magnum-game \
	-G Ninja ..
cmake --build . --target install
