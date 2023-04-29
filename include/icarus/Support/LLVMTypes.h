//
// Created by croemheld on 27.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_SUPPORT_LLVMTYPES_H
#define ICARUS_INCLUDE_ICARUS_SUPPORT_LLVMTYPES_H

#include <llvm/IR/Type.h>

namespace icarus {

/*
 * Utility functions for llvm::Type
 */

unsigned long getNumElements(llvm::Type *T);

llvm::Type *getElementTypeAt(llvm::Type *T, unsigned long N);

/*
 * Utility functions for std::string and llvm::StringRef
 */

unsigned long getAsInteger(llvm::StringRef Str, unsigned int Radix = 10);

} // namespace icarus

#endif // ICARUS_INCLUDE_ICARUS_SUPPORT_LLVMTYPES_H
