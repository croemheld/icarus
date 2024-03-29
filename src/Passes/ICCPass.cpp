//
// Created by croemheld on 26.12.2022.
//

#include <icarus/Support/Clang.h>

#if ICARUS_CLANG_VERSION >= 8
#include <llvm/Analysis/IndirectCallVisitor.h>
#endif

#include <icarus/Passes/ICCPass.h>

namespace icarus::passes {

/*
 * Register pass in icarus
 */

static RegisterPass<ICCPass> ICCPass(nullptr);

/*
 * ICCPass methods
 */

bool ICCPass::checkPassArguments(PassArguments &IPA) {
  return true;
}

int ICCPass::runAnalysisPass(PassArguments &IPA) {
  std::for_each(IPA.begin(), IPA.end(), [&](IcarusModule &IM) {
    llvm::Module *M = IM.getModule();
    for (llvm::Function &F : *M) {
#if ICARUS_CLANG_VERSION >= 8
      IndirectCalls[IM.getFilePath()] += llvm::findIndirectCalls(F).size();
#else
      for (llvm::BasicBlock &BB : F) {
        for (llvm::Instruction &I : BB) {
          if (llvm::CallInst *C = llvm::dyn_cast<llvm::CallInst>(&I)) {
            if (!C->getCalledFunction())
              IndirectCalls[IM.getFilePath()]++;
          }
        }
      }
#endif
    }
  });

  for (auto &[Path, Count] : IndirectCalls) {
    ICARUS_INFO(Path, ": ", Count);
  }

  return 0;
}

} // namespace icarus::passes
