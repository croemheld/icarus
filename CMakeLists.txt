cmake_minimum_required(VERSION 3.9.6)
project(icarus)

# This project requires LLVM to be installed
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(CMAKE_CXX_STANDARD 17)

# Necessary to enable debugger information in CLion
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-3 -glldb")

include_directories(${LLVM_INCLUDE_DIRS} include)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(icarus
        Icarus.cpp
        Logger/Logger.cpp
        Passes/IcarusPass.cpp
        Passes/ICCPass.cpp
        Passes/CPAPass.cpp
        Support/String.cpp
        Threads/ThreadPool.cpp
)

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(icarus ${llvm_libs})