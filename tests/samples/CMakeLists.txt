set(VENV_PATH ${CMAKE_SOURCE_DIR}/venv/bin)
find_program(WLLVM wllvm PATHS ${VENV_PATH})

if (NOT WLLVM)
    message(WARNING "The wllvm binary could not be found, the bitcode files will not be generated. This might lead to failing unit tests that depend on these bitcode files.")
else()
    set(CMAKE_C_COMPILER ${WLLVM})
    set(CMAKE_CXX_COMPILER ${WLLVM}++)

    macro(add_bitcode_test TEST_NAME COMPILER)
        add_custom_command(OUTPUT ${TEST_NAME}.ll
                COMMAND LLVM_COMPILER=clang ${VENV_PATH}/${COMPILER} -c -emit-llvm -S -O0 ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.c
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.c
                COMMENT "Building LLVM IR file ${TEST_NAME}.ll"
                VERBATIM
        )
        add_custom_target(${TEST_NAME} ALL DEPENDS ${TEST_NAME}.ll)
    endmacro()

    macro(add_c_bitcode_test TEST_NAME)
        add_bitcode_test(${TEST_NAME} ${CMAKE_C_COMPILER})
    endmacro()

    macro(add_cpp_bitcode_test TEST_NAME)
        add_bitcode_test(${TEST_NAME} ${CMAKE_CXX_COMPILER})
    endmacro()

    # Add sample C files to targets
    add_c_bitcode_test(constant_propagation)
    add_c_bitcode_test(indirect_calls)
endif (NOT WLLVM)
