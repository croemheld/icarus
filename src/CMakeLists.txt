set(PROJECT_EXECUTABLE ${PROJECT_NAME})

# Extend CMAKE_MODULE_PATH for reusable CMake scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

# This project requires LLVM to be installed
find_package(LLVM ${LLVM_VERSION} REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# C++ standard, RTTI-specific settings, debugging symbols
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-rtti -gdwarf-3 -glldb")

# Add LLVM include directories for linking against libraries
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Add target-specific includes and sources
add_executable(${PROJECT_EXECUTABLE})
target_include_directories(${PROJECT_EXECUTABLE} PRIVATE ../include)

# Add all subdirectories of the project
add_subdirectory(Analysis)
add_subdirectory(Logger)
add_subdirectory(Passes)
add_subdirectory(Support)
add_subdirectory(Threads)

# Add entrypoint (main) Icarus.cpp file
target_sources(${PROJECT_EXECUTABLE} PRIVATE Icarus.cpp)

# The libraries are intentionally listed in the following way to ensure that we
# can extract all LLVM libraries via the script used to build docker images (in
# docker/build-llvm.sh). This allows us to generate minimum-sized images, which
# only contain LLVM libraries needed for building this project.
include(LLVMLibraries)
target_link_libraries(${PROJECT_EXECUTABLE} ${LLVM_LIBRARIES})

# The following libraries do not belong to the LLVM toolchain, but are required
# to generate a fully working version of this project.
target_link_libraries(${PROJECT_EXECUTABLE} z rt dl pthread m)