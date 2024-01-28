# Minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.13.0)

# Define the project name, and thus the executable.
set(PROJECT_NAME icarus)
set(LIBRARY_NAME libIcarus)
project(${PROJECT_NAME})

# Options to enable if so wished.
option(OPTION_BUILD_DOCS "Build documentation." OFF)
option(OPTION_BUILD_LIBS "Build project libraries." OFF)
include(CMakeDependentOption)
cmake_dependent_option(OPTION_BUILD_TEST "Build unit tests." ON "OPTION_BUILD_LIBS" OFF)

# Options for compiling with coverage instrumentation
option(OPTION_BUILD_COVERAGE "Build with coverage instrumentation." OFF)

# Extend CMAKE_MODULE_PATH for reusable CMake scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# This project requires LLVM to be installed
find_package(LLVM ${LLVM_VERSION} REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# C++ standard, RTTI-specific settings, debugging symbols
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-caret-diagnostics -fno-rtti")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-3 -glldb")

# Fix clang-tidy warning for system headers

# Add coverage instrumentation flags if enabled (currently Clang only)
if (OPTION_BUILD_COVERAGE)
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate -fcoverage-mapping)
endif (OPTION_BUILD_COVERAGE)

# Add LLVM include directories for linking against libraries
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# The libraries are intentionally listed in the following way to ensure that we
# can extract all LLVM libraries via the script used to build docker images (in
# docker/build-llvm.sh). This allows us to generate minimum-sized images, which
# only contain LLVM libraries needed for building this project.
include(LLVMLibraries)

# The following libraries do not belong to the LLVM toolchain, but are required
# to generate a fully working version of this project.
set(EXTERNAL_LIBRARIES z rt dl pthread m)

# Project and tests include directory
set(PROJECT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(DOCTEST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/tests/include")

# Add main target for project executable
add_executable(${PROJECT_NAME})

# Source code directory
add_subdirectory(src)

# Add library target if option has been enabled
if (OPTION_BUILD_LIBS)
    get_target_property(LIBRARY_SOURCES ${PROJECT_NAME} SOURCES)
    add_library(${LIBRARY_NAME} STATIC ${LIBRARY_SOURCES})
    target_include_directories(${LIBRARY_NAME} PRIVATE ${PROJECT_INCLUDE_DIR})
    set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME Icarus)
    target_link_libraries(${LIBRARY_NAME}
            ${LLVM_LIBRARIES}
            ${EXTERNAL_LIBRARIES}
    )
endif (OPTION_BUILD_LIBS)

# Include unit tests using the doctest framework.
if (OPTION_BUILD_TEST AND TARGET ${LIBRARY_NAME})
    enable_testing()
    add_subdirectory(tests)
endif (OPTION_BUILD_TEST AND TARGET ${LIBRARY_NAME})

# Search for doxygen and include docs directory.
if (OPTION_BUILD_DOCS)
    find_package(Doxygen)
    add_subdirectory(docs)
endif (OPTION_BUILD_DOCS)

# We place the remaining instructions for the project executable here, in order
# to not add the additional Icarus.cpp file to the other target's sources. That
# way, we can use the currently added source files to build the project library
# first that the unit test executable can link against.

# Add target-specific includes and sources, link against LLVM and other external
# libraries needed for the project executable.
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIR})
target_sources(${PROJECT_NAME} PRIVATE src/Icarus.cpp)
target_link_libraries(${PROJECT_NAME} ${LLVM_LIBRARIES} ${EXTERNAL_LIBRARIES})
