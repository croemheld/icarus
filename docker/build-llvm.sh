#!/usr/bin/env sh

set -e

LLVM_CHECKOUT="${1}"

CMAKE_ADDRESS="$(pwd)/llvm/CMakeLists.txt"

PARALLEL_JOBS="${2}"

git checkout "${LLVM_CHECKOUT}"

LLVM_MAJOR_VERSION="$(grep 'LLVM_VERSION_MAJOR ' "${CMAKE_ADDRESS}" | awk -F'[()]' '{print $2}' | cut -d ' ' -f2)"

#######################################
# LLVM patches that were not released #
#######################################

if [ "${LLVM_MAJOR_VERSION}" -le "11" ]; then
  # List of patches for versions 5 to 11 (not ordered):
  #
  # https://bugzilla.redhat.com/attachment.cgi?id=1389687&action=diff
  # https://reviews.llvm.org/D47281
  # https://reviews.llvm.org/D70662
  # https://reviews.llvm.org/D64937
  # https://reviews.llvm.org/D89450
  # Fix for unknown 'size_t' by replacing with 'std::size_t' in xray_buffer_queue.h
  #
  # These patches make it possible to compile LLVM with compiler-rt for Ubuntu Jammy.
  patch -p1 < patches/llvm-"${LLVM_MAJOR_VERSION}".patch
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
  -DCMAKE_CXX_FLAGS="-w" \
  -S llvm -B build

cmake --build build
