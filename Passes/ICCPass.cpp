//
// Created by croemheld on 26.12.2022.
//

#include <llvm/Analysis/IndirectCallVisitor.h>

#include <icarus/Passes/ICCPass.h>

namespace icarus {

/*
 * Register pass in icarus
 */

static RegisterPass<ICCPass> ICCPass(nullptr);

/*
 * ICCPass methods
 */

int ICCPass::runAnalysisPass(IcarusPassArguments &IPA) {
  forEachModule(IPA, [&](IcarusModule &IM) {
    llvm::Module *M = IM.getModule();
    for (llvm::Function &F : *M) {
      IndirectCalls[IM.getFilePath()] += llvm::findIndirectCalls(F).size();
    }
  });

  for (auto &[Path, Count] : IndirectCalls) {
    INFO(Path, ": ", Count);
  }

  return 0;
}

}