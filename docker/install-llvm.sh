#!/usr/bin/env sh

set -e

# Some files need to be patched in order to allow for an incomplete installation of LLVM and Clang.
# We use this function to patch these here. We also should not use sed -i as the option is not part
# of all implementations and might lead to unrecognized or erroneous options.
patch_cmake_include() {
  sed "s/include(\"\${${1}}\/${2}\")/# &/" "${3}" > out.cmake
  mv out.cmake "${3}"
}

CMAKE_ADDRESS="$(pwd)/llvm/CMakeLists.txt"
LLVM_MAJORVER="$(grep 'LLVM_VERSION_MAJOR ' "${CMAKE_ADDRESS}" | awk -F'[()]' '{print $2}' | cut -d ' ' -f2)"
CMAKE_TARGETS="${*}"

if [ "${CMAKE_TARGETS}" = "all" ]; then
  NINJA_INSTALL="install"
else
  #######################################
  # Version > 8 does not include stddef #
  # or any builtin headers, add target! #
  #######################################

  NINJA_TARGETS="clang clang-tidy clang-format clang-headers llvm-headers llvm-config cmake-exports"

  if [ "${#CMAKE_TARGETS[@]}" -gt "0" ]; then
    NINJA_TARGETS="${CMAKE_TARGETS} ${NINJA_TARGETS}"
  fi

  if [ "${LLVM_MAJORVER}" -gt "8" ]; then
    NINJA_TARGETS="${NINJA_TARGETS} clang-resource-headers clang-cmake-exports"
  fi

  #######################################
  # Only install minimum LLVM libraries #
  #######################################

  LIBRARY_NAMES=$(cmake -DLLVM_PACKAGE_VERSION="${LLVM_MAJORVER}" -P ../LLVMLibraries.cmake 2>&1 | xargs -d ';')
  NINJA_INSTALL="$(printf '%s %s' "${NINJA_TARGETS}" "${LIBRARY_NAMES}" | sed 's/[^ ]* */install-&/g')"
fi

#######################################
# Build projects with CMake arguments #
#######################################

cd build && echo "Install targets: ${NINJA_INSTALL}" && ninja ${NINJA_INSTALL}

if [ "${CMAKE_TARGETS}" != "all" ]; then
  #######################################
  # Comment include of LLVMExports file #
  #######################################

  # We comment the include of the LLVMExports.cmake file in order to generate a docker image with the
  # bare minimum of libraries that are needed for this project. If we include that file, we are going
  # to get a CMake error saying that the LLVM libraries were not installed completely.
  patch_cmake_include "LLVM_CMAKE_DIR" "LLVMExports.cmake" "/usr/local/lib/cmake/llvm/LLVMConfig.cmake"
  if [ "${LLVM_MAJORVER}" -gt "8" ]; then
    patch_cmake_include "CLANG_CMAKE_DIR" "ClangTargets.cmake" "/usr/local/lib/cmake/clang/ClangConfig.cmake"
  fi
fi

#######################################
# Remove build directory to get space #
#######################################

cd .. && rm -rf build

#######################################
# Compile compiler-rt with LLVM build #
#######################################

mkdir -p build-compiler-rt && cd build-compiler-rt
cmake -G "Ninja" \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DLLVM_CONFIG_PATH=/usr/local/bin/llvm-config \
  ../compiler-rt
ninja install

cd .. && rm -rf build-compiler-rt
