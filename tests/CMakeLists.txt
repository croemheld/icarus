add_executable(TestIcarus TestIcarus.cpp)

# Helper macro for adding doctest file to TestIcarus
macro(add_doctest_test TEST_NAME)
    target_sources(TestIcarus PUBLIC ${TEST_NAME}.cpp)
endmacro(add_doctest_test)

# Add unit test subdirectories
add_subdirectory(TestADT)
add_subdirectory(TestPasses)

# Specify target properties
target_include_directories(TestIcarus PUBLIC ${PROJECT_INCLUDE_DIR})
target_include_directories(TestIcarus SYSTEM PRIVATE ${DOCTEST_INCLUDE_DIR})
target_link_libraries(TestIcarus ${LIBRARY_NAME})
target_compile_definitions(TestIcarus PUBLIC -DSAMPLES_DIR="${CMAKE_CURRENT_BINARY_DIR}/samples")
add_test(NAME TestIcarus COMMAND TestIcarus)
