# Doxygen availability was checked in root CMakeLists file
if (DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(DOXYGEN_PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(DOXYGEN_PROJECT_DATABASE_PATH ${CMAKE_BINARY_DIR})

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen.cfg.in
            ${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg @ONLY)

    add_custom_target(doxygen
            COMMAND ${DOXYGEN_EXECUTABLE} -u ${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating ${PROJECT_NAME} doxygen documentation" VERBATIM)
endif (DOXYGEN_FOUND)
