# Base libraries that are required to be linked against for icarus.
set(LLVM_LIBRARIES
    LLVMIRReader
    LLVMAsmParser
    LLVMBitReader
    LLVMCore
    LLVMBinaryFormat
)

# Further libraries to link against for LLVM versions >= 9.
# The LLVM_PACKAGE_VERSION variable is usually set after the LLVM package was located
# via the find_package command. However, if the CMake file is executed in script mode
# the aforementioned command does not work. To circumvent this issue, we are required
# to set the variable manually via -DLLVM_PACKAGE_VERSION.
if (LLVM_PACKAGE_VERSION GREATER_EQUAL 9)
    list(APPEND LLVM_LIBRARIES
        LLVMBitstreamReader
        LLVMRemarks
    )
endif (LLVM_PACKAGE_VERSION GREATER_EQUAL 9)

# Starting from LLVM 16, llvm::Triple was added to LLVMTargetParser.
if (LLVM_PACKAGE_VERSION GREATER_EQUAL 16)
    list(APPEND LLVM_LIBRARIES
        LLVMTargetParser
    )
endif (LLVM_PACKAGE_VERSION GREATER_EQUAL 16)

# These two libraries should always be linked, because they are used in nearly all of
# the LLVM libraries. Because of link order, they need to be placed at the end.
list(APPEND LLVM_LIBRARIES
    LLVMSupport
    LLVMDemangle
)

# We can call this script from the command line, necessary for docker image creation.
if (NOT CMAKE_PARENT_LIST_FILE)
    message("${LLVM_LIBRARIES}")
endif (NOT CMAKE_PARENT_LIST_FILE)
