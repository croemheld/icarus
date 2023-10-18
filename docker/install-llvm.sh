#!/bin/sh

set -e

CMAKE_ADDRESS="$(pwd)/llvm/CMakeLists.txt"
LLVM_MAJORVER="$(grep 'LLVM_VERSION_MAJOR ' "${CMAKE_ADDRESS}" | awk -F'[()]' '{print $2}' | cut -d ' ' -f2)"
CMAKE_TARGETS="${@}"

#######################################
# Version > 8 does not include stddef #
# or any builtin headers, add target! #
#######################################

if [ "${LLVM_MAJORVER}" -gt "8" ]; then
	CMAKE_TARGETS="${CMAKE_TARGETS} install-clang-resource-headers"
fi

#######################################
# Only install minimum LLVM libraries #
#######################################

BUILD_TARGETS=$(cmake -DLLVM_PACKAGE_VERSION="${LLVM_MAJORVER}" -P ../LLVMLibraries.cmake 2>&1 | xargs -d ';')
CMAKE_TARGETS="${CMAKE_TARGETS} $(echo "${BUILD_TARGETS}" | sed 's/[^ ]* */install-&/g')"

#######################################
# Build projects with CMake arguments #
#######################################

cd build && echo "Install targets: ${CMAKE_TARGETS}" && ninja $CMAKE_TARGETS

#######################################
# Comment include of LLVMExports file #
#######################################

# Do not use sed -i as this is not always supported or handled correctly
sed 's/include(\"${LLVM_CMAKE_DIR}\/LLVMExports.cmake\")/# &/' /usr/local/lib/cmake/llvm/LLVMConfig.cmake > out.cmake
mv out.cmake /usr/local/lib/cmake/llvm/LLVMConfig.cmake
