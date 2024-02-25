#!/usr/bin/env sh

set -e

LLVM_CHECKOUT="${1}"

CMAKE_ADDRESS="$(pwd)/llvm/CMakeLists.txt"

PARALLEL_JOBS="${2}"

git pull && git checkout "${LLVM_CHECKOUT}"

LLVM_MAJOR_VERSION="$(grep 'LLVM_VERSION_MAJOR ' "${CMAKE_ADDRESS}" | awk -F'[()]' '{print $2}' | cut -d ' ' -f2)"

if [ "${LLVM_MAJOR_VERSION}" = "8" ]; then
  patch -p1 < MicrosoftDemangleNodes.patch
fi

#######################################
# Build projects with CMake arguments #
#######################################

cmake -G "Ninja" \
  -DCMAKE_BUILD_TYPE=Release \
  -DLIBCLANG_BUILD_STATIC=ON \
  -DBUILD_SHARED_LIBS=OFF \
  -DLLVM_ENABLE_PIC=OFF \
  -DLLVM_BUILD_LLVM_DYLIB=OFF \
  -DLLVM_BUILD_LLVM_C_DYLIB=OFF \
  -DLLVM_ENABLE_TERMINFO=OFF \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
  -DLLVM_TARGETS_TO_BUILD="X86" \
  -DLLVM_PARALLEL_COMPILE_JOBS="${PARALLEL_JOBS}" \
  -DLLVM_PARALLEL_LINK_JOBS="1" \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++ \
  -S llvm -B build

cmake --build build
