#!/usr/bin/env sh

set -e

LLVM_CHECKOUT="$1"

BUILD_ADDRESS="$(pwd)/build"
CMAKE_ADDRESS="$(pwd)/llvm/CMakeLists.txt"

PARALLEL_JOBS="$2"

git checkout "$LLVM_CHECKOUT"

LLVM_MAJORVER="$(grep 'LLVM_VERSION_MAJOR ' "$CMAKE_ADDRESS" | awk -F'[()]' '{print $2}' | cut -d ' ' -f2)"

#######################################
# Build projects with CMake arguments #
#######################################

cmake -G "Ninja" \
	-DCMAKE_BUILD_TYPE=Release \
	-DLLVM_ENABLE_PROJECTS="clang" \
  -DLLVM_TARGETS_TO_BUILD="X86" \
  -DLLVM_PARALLEL_COMPILE_JOBS="$PARALLEL_JOBS" \
  -DLLVM_PARALLEL_LINK_JOBS="1" \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++ \
  -S llvm -B build

echo "Build llvm-project (major: $LLVM_MAJORVER) in: $BUILD_ADDRESS"
cmake --build build
