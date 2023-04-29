//
// Created by croemheld on 28.01.2023.
//

#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>

#include "icarus/Support/Clang.h"
#include "icarus/Support/LLVMValue.h"

namespace icarus {

std::vector<llvm::BasicBlock *> getBasicBlocks(llvm::Function &F) {
  return getBasicBlocks(F, [](llvm::BasicBlock &BB) { return true; });
}

std::vector<llvm::BasicBlock *> getExitBlocks(llvm::Function &F) {
  return getBasicBlocks(F, [](llvm::BasicBlock &BB) {
#if ICARUS_CLANG_VERSION >= 7
    return !llvm::succ_size(&BB);
#else
    return !std::distance(llvm::succ_begin(&BB), llvm::succ_end(&BB));
#endif
  });
}

llvm::BasicBlock *getExitBlock(llvm::Function &F) {
  std::vector<llvm::BasicBlock *> ExitBlocks = getExitBlocks(F);
  return ExitBlocks.size() == 1 ? ExitBlocks.front() : nullptr;
}

llvm::Constant *getConstant(llvm::Type *T) {
  return llvm::UndefValue::get(T);
}

llvm::Constant *getConstant(llvm::Type *T, llvm::ArrayRef<llvm::Constant *> Elements) {
  if (T->isArrayTy()) {
    return getConstant<llvm::ArrayType, llvm::ConstantArray>(T, Elements);
  } else if (T->isStructTy()) {
    return getConstant<llvm::StructType, llvm::ConstantStruct>(T, Elements);
  }
  return nullptr;
}

} // namespace icarus