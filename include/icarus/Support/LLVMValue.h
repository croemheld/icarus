//
// Created by croemheld on 28.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_SUPPORT_LLVMVALUE_H
#define ICARUS_INCLUDE_ICARUS_SUPPORT_LLVMVALUE_H

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>

namespace icarus {

/*
 * llvm::BasicBlock utility methods
 */

/**
 * Collects all basic blocks of a function matching the provided predicate.
 * @tparam PredicateT The predicate type.
 * @param F The llvm::Function instance from which to collect basic blocks.
 * @param Predicate The predicate to be fulfilled by a basic block.
 * @return A std::vector containing pointers to all matching basic blocks.
 */
template <typename PredicateT>
std::vector<llvm::BasicBlock *> getBasicBlocks(llvm::Function &F, PredicateT Predicate) {
  std::vector<llvm::BasicBlock *> BasicBlocks;
  for (llvm::BasicBlock &BB : F) {
    if (Predicate(BB)) {
      BasicBlocks.push_back(&BB);
    }
  }
  return BasicBlocks;
}

/**
 * Collects all basic blocks of a function.
 * @param F The llvm::Function instance from which to collect basic blocks.
 * @return A std::vector containing all basic blocks of the function.
 */
std::vector<llvm::BasicBlock *> getBasicBlocks(llvm::Function &F);

/**
 * Collects all exit basic blocks of a function.
 * @param F The llvm::Function instance from which to collect basic blocks.
 * @return A std::vector containing all exit basic blocks of the function.
 */
std::vector<llvm::BasicBlock *> getExitBlocks(llvm::Function &F);

/**
 * @param F The llvm::Function instance from which to collect basic blocks.
 * @return Returns the sole exit basic block or nullptr, if not found.
 */
llvm::BasicBlock *getUniqueExitBlock(llvm::Function &F);

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
