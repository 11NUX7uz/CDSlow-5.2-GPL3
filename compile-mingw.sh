#!/bin/sh

BUILD_TYPE='Release'
CMAKE_MINGW='-D CMAKE_TOOLCHAIN_FILE=/usr/share/mingw/toolchain-mingw32.cmake'
BUILD_DIR='build'

test -d "$BUILD_DIR" || mkdir "$BUILD_DIR"
cd "$BUILD_DIR" || exit

cmake $CMAKE_MINGW -D CMAKE_BUILD_TYPE=${BUILD_TYPE} .. && cmake --build .
