# Helper macro for adding a single unit test
macro(add_doctest_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_include_directories(${TEST_NAME} PRIVATE ${PROJECT_INCLUDE_DIR})
    target_include_directories(${TEST_NAME} SYSTEM PRIVATE ${DOCTEST_INCLUDE_DIR})
    target_link_libraries(${TEST_NAME} ${LIBRARY_NAME})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro(add_doctest_test)

# Add unit test subdirectories
add_subdirectory(TestADT)
