# Add unit test subdirectories
add_subdirectory(TestADT)

target_sources(${TESTING_NAME} PRIVATE TestIcarus.cpp)