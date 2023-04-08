cmake_minimum_required(VERSION 3.9.6)
project(icarus)

# This project requires LLVM to be installed
find_package(LLVM ${LLVM_VERSION} REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(CMAKE_CXX_STANDARD 17)

# Disable C++ RTTI, we rely on LLVM's RTTI system
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

# Necessary to enable debugger information in CLion
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-rtti -gdwarf-3 -glldb")

include_directories(${LLVM_INCLUDE_DIRS} include)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(icarus
        Icarus.cpp
        Logger/Logger.cpp
        Passes/Pass.cpp
        Passes/PassArguments.cpp
        Passes/IAAPass.cpp
        Passes/ICCPass.cpp
        Passes/CPAPass.cpp
        Support/LLVMTypes.cpp
        Support/LLVMValue.cpp
        Support/String.cpp
        Threads/ThreadPool.cpp
        Analysis/EngineValue.cpp
        Analysis/ExecutionEngine.cpp
        )

# The libraries are intentionally listed in the following way to ensure that we
# can extract all LLVM libraries via the script used to build docker images (in
# docker/build-llvm.sh). This allows us to generate minimum-sized images, which
# only contain LLVM libraries needed for building this project.
target_link_libraries(icarus
        LLVMIRReader
        LLVMAsmParser
        LLVMBitReader
        LLVMCore
        LLVMBinaryFormat
        LLVMSupport
        LLVMDemangle
        )

# Further libraries to link against for LLVM versions >= 9.
if (LLVM_PACKAGE_VERSION GREATER_EQUAL 9)
    target_link_libraries(icarus
            LLVMBitstreamReader
            LLVMRemarks
            )
endif (LLVM_PACKAGE_VERSION GREATER_EQUAL 9)

# The following libraries do not belong to the LLVM toolchain, but are required
# to generate a fully working version of this project.
target_link_libraries(icarus z rt dl pthread m)