#!/bin/bash -ev

cd "$(dirname "$0")" 

brew install eigen wget ninja glm emscripten

mkdir -p emscripten-dependencies-src && cd emscripten-dependencies-src 


test -d corrade || git clone --depth 1 https://github.com/mosra/corrade.git
(
	cd corrade
	git pull

	# Build native corrade-rc
	mkdir -p build && cd build || exit /b
	cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=$HOME/deps-native \
		-DCORRADE_WITH_INTERCONNECT=OFF \
		-DCORRADE_WITH_PLUGINMANAGER=OFF \
		-DCORRADE_WITH_TESTSUITE=OFF \
		-DCORRADE_WITH_UTILITY=OFF \
		-G Ninja
	ninja install
	cd ..

	# Crosscompile Corrade
	mkdir -p build-emscripten && cd build-emscripten
	cmake .. \
		-DCMAKE_TOOLCHAIN_FILE="../../toolchains/generic/Emscripten-wasm.cmake" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG -O1" \
		-DCMAKE_EXE_LINKER_FLAGS_RELEASE="-O1" \
		-DCMAKE_INSTALL_PREFIX=$HOME/deps \
		-DCORRADE_RC_EXECUTABLE=$HOME/deps-native/bin/corrade-rc \
		-DCORRADE_WITH_INTERCONNECT=OFF \
		-G Ninja
	ninja install
)



test -e 2.87.tar.gz || wget https://github.com/bulletphysics/bullet3/archive/2.87.tar.gz
tar -xzf 2.87.tar.gz && (

	cd bullet3-2.87
	mkdir -p build-emscripten && cd build-emscripten
	cmake .. \
		-DCMAKE_TOOLCHAIN_FILE="../../toolchains/generic/Emscripten-wasm.cmake" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG -O1" \
		-DCMAKE_EXE_LINKER_FLAGS_RELEASE="-O1" \
		-DCMAKE_INSTALL_PREFIX=$HOME/deps \
		-DBUILD_BULLET2_DEMOS=OFF \
		-DBUILD_BULLET3=OFF \
		-DBUILD_CLSOCKET=OFF \
		-DBUILD_CPU_DEMOS=OFF \
		-DBUILD_ENET=OFF \
		-DBUILD_EXTRAS=OFF \
		-DBUILD_OPENGL3_DEMOS=OFF \
		-DBUILD_PYBULLET=OFF \
		-DBUILD_UNIT_TESTS=OFF \
		-DINSTALL_LIBS=ON \
		-DINSTALL_CMAKE_FILES=OFF \
		-DUSE_GLUT=OFF \
		-DUSE_GRAPHICAL_BENCHMARK=OFF \
		-D_FIND_LIB_PYTHON_PY=$TRAVIS_BUILD_DIR/bullet3-2.87/build3/cmake/FindLibPython.py \
		-G Ninja
	ninja install

)

test -d magnum || git clone https://github.com/mosra/magnum.git
(
	cd magnum
	git pull
	mkdir -p build-emscripten && cd build-emscripten
	cmake .. \
		-DCMAKE_TOOLCHAIN_FILE="../../toolchains/generic/Emscripten-wasm.cmake" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG -O1" \
		-DCMAKE_EXE_LINKER_FLAGS_RELEASE="-O1" \
		-DCMAKE_INSTALL_PREFIX=$HOME/deps \
		-DCMAKE_FIND_ROOT_PATH=$HOME/deps \
		-DCMAKE_PREFIX_PATH=/opt/homebrew \
		-DCORRADE_RC_EXECUTABLE=$HOME/deps-native/bin/corrade-rc \
		-DMAGNUM_WITH_AUDIO=ON \
		-DMAGNUM_WITH_DEBUGTOOLS=ON \
		-DMAGNUM_WITH_MATERIALTOOLS=OFF \
		-DMAGNUM_WITH_MESHTOOLS=ON \
		-DMAGNUM_WITH_PRIMITIVES=ON \
		-DMAGNUM_WITH_SCENEGRAPH=ON \
		-DMAGNUM_WITH_SCENETOOLS=OFF \
		-DMAGNUM_WITH_SHADERS=ON \
		-DMAGNUM_WITH_TEXT=OFF \
		-DMAGNUM_WITH_TEXTURETOOLS=OFF \
		-DMAGNUM_WITH_OPENGLTESTER=ON \
		-DMAGNUM_WITH_EMSCRIPTENAPPLICATION=ON \
		-DMAGNUM_WITH_WINDOWLESSEGLAPPLICATION=ON \
		-DMAGNUM_TARGET_GLES2=$TARGET_GLES2 \
		-G Ninja
	ninja install	
)

test -d magnum-integration || git clone https://github.com/mosra/magnum-integration.git
(
	cd magnum-integration
	git pull
	git submodule update --init

	mkdir -p build-emscripten && cd build-emscripten
	cmake .. \
		-DCMAKE_TOOLCHAIN_FILE="../toolchains/generic/Emscripten-wasm.cmake" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG -O1" \
		-DCMAKE_EXE_LINKER_FLAGS_RELEASE="-O1" \
		-DCMAKE_PREFIX_PATH=/opt/homebrew \
		-DCMAKE_INSTALL_PREFIX=$HOME/deps \
		-DCMAKE_FIND_ROOT_PATH=$HOME/deps \
		-DGLM_INCLUDE_DIR=$HOME/glm \
		-DIMGUI_DIR=$HOME/imgui \
		-DCORRADE_RC_EXECUTABLE=$HOME/deps-native/bin/corrade-rc \
		-DMAGNUM_WITH_BULLET=ON \
		-DMAGNUM_WITH_DART=OFF \
		-DMAGNUM_WITH_EIGEN=OFF \
		-DMAGNUM_WITH_GLM=OFF \
		-DMAGNUM_WITH_IMGUI=OFF \
		-DMAGNUM_WITH_OVR=OFF \
		-DMAGNUM_BUILD_TESTS=ON \
		-DMAGNUM_BUILD_GL_TESTS=ON \
		-G Ninja
	ninja $NINJA_JOBS	
	ninja install
)

# Crosscompile zstd. Version 1.5.1+ doesn't compile with this Emscripten
# version, saying that
#   huf_decompress_amd64.S: Input file has an unknown suffix, don't know what to do with it!
# Newer Emscriptens work fine, 1.5.0 doesn't have this file yet so it works.
export ZSTD_VERSION=1.5.0
test -e v$ZSTD_VERSION.tar.gz || wget --no-check-certificate https://github.com/facebook/zstd/archive/refs/tags/v$ZSTD_VERSION.tar.gz
tar -xzf v$ZSTD_VERSION.tar.gz && (
	cd zstd-$ZSTD_VERSION
	# There's already a directory named `build`
	mkdir -p build_ && cd build_
	cmake ../build/cmake \
		-DCMAKE_TOOLCHAIN_FILE="../../magnum-integration/toolchains/generic/Emscripten-wasm.cmake" \
		-DCMAKE_BUILD_TYPE=Debug \
		-DZSTD_BUILD_PROGRAMS=OFF \
		-DZSTD_BUILD_SHARED=OFF \
		-DZSTD_BUILD_STATIC=ON \
		-DZSTD_MULTITHREAD_SUPPORT=OFF \
		-DCMAKE_INSTALL_PREFIX=$HOME/deps \
		-G Ninja
	ninja install
)

# Crosscompile OpenEXR
export OPENEXR_VERSION=3.2.0
test -e openexr-$OPENEXR_VERSION.tar.gz || wget --no-check-certificate https://github.com/AcademySoftwareFoundation/openexr/archive/v$OPENEXR_VERSION/openexr-$OPENEXR_VERSION.tar.gz
tar -xzf openexr-$OPENEXR_VERSION.tar.gz && (
	cd openexr-$OPENEXR_VERSION
	mkdir -p build && cd build
	cmake .. \
		-DCMAKE_TOOLCHAIN_FILE="../../magnum-integration/toolchains/generic/Emscripten-wasm.cmake" \
		-DCMAKE_INSTALL_PREFIX=$HOME/deps \
		-DOPENEXR_BUILD_TOOLS=OFF \
		-DOPENEXR_ENABLE_THREADING=OFF \
		-DBUILD_TESTING=OFF \
		-DBUILD_SHARED_LIBS=OFF \
		-DOPENEXR_INSTALL_EXAMPLES=OFF \
		-DOPENEXR_INSTALL_TOOLS=OFF \
		-DOPENEXR_INSTALL_PKG_CONFIG=OFF \
		-DOPENEXR_FORCE_INTERNAL_IMATH=ON \
		-DOPENEXR_FORCE_INTERNAL_DEFLATE=ON \
		`# v1.18 (which is the default) has different output and the test files` \
		`# are made against v1.19 now` \
		-DOPENEXR_DEFLATE_TAG=v1.19 \
		-DIMATH_INSTALL_PKG_CONFIG=OFF \
		-DIMATH_HALF_USE_LOOKUP_TABLE=OFF \
		-G Ninja
	ninja install
)

test -d magnum-plugins || git clone https://github.com/mosra/magnum-plugins.git
(
	cd magnum-plugins

	git submodule update --init

	mkdir -p build-emscripten && cd build-emscripten
	cmake .. \
		-DCMAKE_TOOLCHAIN_FILE="../toolchains/generic/Emscripten-wasm.cmake" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG -O1" \
		-DCMAKE_EXE_LINKER_FLAGS_RELEASE="-O1" \
		-DCMAKE_INSTALL_PREFIX=$HOME/deps \
		-DCMAKE_FIND_ROOT_PATH=$HOME/deps \
		-DCORRADE_RC_EXECUTABLE=$HOME/deps-native/bin/corrade-rc \
		-DMAGNUM_WITH_ASSIMPIMPORTER=OFF \
		-DMAGNUM_WITH_ASTCIMPORTER=ON \
		-DMAGNUM_WITH_BASISIMAGECONVERTER=OFF \
		-DMAGNUM_WITH_BASISIMPORTER=OFF -DBASIS_UNIVERSAL_DIR=$HOME/basis_universal \
		-DMAGNUM_WITH_BCDECIMAGECONVERTER=ON \
		-DMAGNUM_WITH_CGLTFIMPORTER=ON \
		-DMAGNUM_WITH_DDSIMPORTER=ON \
		-DMAGNUM_WITH_DEVILIMAGEIMPORTER=OFF \
		-DMAGNUM_WITH_DRFLACAUDIOIMPORTER=ON \
		-DMAGNUM_WITH_DRMP3AUDIOIMPORTER=ON \
		-DMAGNUM_WITH_DRWAVAUDIOIMPORTER=ON \
		-DMAGNUM_WITH_ETCDECIMAGECONVERTER=ON \
		-DMAGNUM_WITH_FAAD2AUDIOIMPORTER=OFF \
		-DMAGNUM_WITH_FREETYPEFONT=OFF \
		-DMAGNUM_WITH_GLSLANGSHADERCONVERTER=OFF \
		-DMAGNUM_WITH_GLTFIMPORTER=ON \
		-DMAGNUM_WITH_GLTFSCENECONVERTER=ON \
		-DMAGNUM_WITH_HARFBUZZFONT=OFF \
		-DMAGNUM_WITH_ICOIMPORTER=ON \
		-DMAGNUM_WITH_JPEGIMAGECONVERTER=OFF \
		-DMAGNUM_WITH_JPEGIMPORTER=OFF \
		-DMAGNUM_WITH_KTXIMAGECONVERTER=ON \
		-DMAGNUM_WITH_KTXIMPORTER=ON \
		-DMAGNUM_WITH_MESHOPTIMIZERSCENECONVERTER=OFF \
		-DMAGNUM_WITH_MINIEXRIMAGECONVERTER=ON \
		-DMAGNUM_WITH_OPENEXRIMAGECONVERTER=ON \
		-DMAGNUM_WITH_OPENEXRIMPORTER=ON \
		-DMAGNUM_WITH_OPENGEXIMPORTER=ON \
		-DMAGNUM_WITH_PNGIMAGECONVERTER=OFF \
		-DMAGNUM_WITH_PNGIMPORTER=OFF \
		-DMAGNUM_WITH_PRIMITIVEIMPORTER=ON \
		-DMAGNUM_WITH_SPIRVTOOLSSHADERCONVERTER=OFF \
		-DMAGNUM_WITH_SPNGIMPORTER=OFF \
		-DMAGNUM_WITH_STANFORDIMPORTER=ON \
		-DMAGNUM_WITH_STANFORDSCENECONVERTER=ON \
		-DMAGNUM_WITH_STBDXTIMAGECONVERTER=ON \
		-DMAGNUM_WITH_STBIMAGECONVERTER=ON \
		-DMAGNUM_WITH_STBIMAGEIMPORTER=ON \
		-DMAGNUM_WITH_STBRESIZEIMAGECONVERTER=ON \
		-DMAGNUM_WITH_STBTRUETYPEFONT=ON \
		-DMAGNUM_WITH_STBVORBISAUDIOIMPORTER=ON \
		-DMAGNUM_WITH_STLIMPORTER=ON \
		-DMAGNUM_WITH_TINYGLTFIMPORTER=ON \
		-DMAGNUM_WITH_UFBXIMPORTER=ON \
		-DMAGNUM_WITH_WEBPIMAGECONVERTER=OFF \
		-DMAGNUM_WITH_WEBPIMPORTER=OFF \
		-DMAGNUM_BUILD_TESTS=ON \
		-DMAGNUM_BUILD_GL_TESTS=ON \
		-G Ninja
	ninja $NINJA_JOBS

	# Test
	#CORRADE_TEST_COLOR=ON ctest -V

	# Test install, after running the tests as for them it shouldn't be needed
	ninja install
)
