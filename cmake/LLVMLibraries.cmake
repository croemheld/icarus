# Base libraries that are required to be linked against for icarus.
set(LLVM_LIBRARIES
        LLVMIRReader
        LLVMAsmParser
        LLVMBitReader
        LLVMCore
        LLVMBinaryFormat
        LLVMSupport
        LLVMDemangle
        CACHE INTERNAL ""
)

# Further libraries to link against for LLVM versions >= 9.
if (LLVM_PACKAGE_VERSION GREATER_EQUAL 9)
    list(APPEND LLVM_LIBRARIES
            LLVMBitstreamReader
            LLVMRemarks
    )
endif (LLVM_PACKAGE_VERSION GREATER_EQUAL 9)

# We can call this script from the command line, necessary for docker image creation.
if (NOT CMAKE_PARENT_LIST_FILE)
    message("${LLVM_LIBRARIES}")
endif (NOT CMAKE_PARENT_LIST_FILE)