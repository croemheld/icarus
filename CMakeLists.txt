# Minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.13.0)

# Define the project name, and thus the executable.
set(PROJECT_NAME icarus)
project(${PROJECT_NAME})

option(OPTION_BUILD_DOCS "Build documentation." OFF)

# Source code directory
add_subdirectory(src)

# Search for doxygen and include docs directory.
if (OPTION_BUILD_DOCS)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif (DOXYGEN_FOUND)
endif (OPTION_BUILD_DOCS)