//
// Created by croemheld on 28.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_SUPPORT_LLVMVALUE_H
#define ICARUS_INCLUDE_ICARUS_SUPPORT_LLVMVALUE_H

#include <llvm/IR/Constants.h>

namespace icarus {

/*
 * llvm::Constant utility methods
 */

llvm::Constant *getConstant(llvm::Type *T);

template <typename AT, typename AC>
llvm::Constant *getConstant(llvm::Type *T, llvm::ArrayRef<llvm::Constant *> Elements) {
  return AC::get(llvm::dyn_cast<AT>(T), Elements);
}

llvm::Constant *getConstant(llvm::Type *T, llvm::ArrayRef<llvm::Constant *> Elements);

}

#endif // ICARUS_INCLUDE_ICARUS_SUPPORT_LLVMVALUE_H
