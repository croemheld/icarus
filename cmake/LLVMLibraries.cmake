# Base libraries that are required to be linked against for icarus.
set(LLVM_LIBRARIES
        LLVMIRReader
        LLVMAsmParser
        LLVMBitReader
        LLVMSupport
        LLVMDemangle
        CACHE INTERNAL ""
)

# Conclude LLVM libraries with LLVMCore
list(APPEND LLVM_LIBRARIES LLVMCore)

# We can call this script from the command line, necessary for docker image creation.
if (NOT CMAKE_PARENT_LIST_FILE)
    message("${LLVM_LIBRARIES}")
endif (NOT CMAKE_PARENT_LIST_FILE)
