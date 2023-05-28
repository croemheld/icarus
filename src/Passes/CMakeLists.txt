set(SOURCES
        CPAPass.cpp
        IAAPass.cpp
        ICCPass.cpp
        Pass.cpp
        PassArguments.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})