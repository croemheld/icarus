//
// Created by croemheld on 26.12.2022.
//

#include <llvm/IR/Value.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>

#include <icarus/Passes/ICCPass.h>

namespace icarus {

/*
 * Register pass in icarus
 */

static RegisterPass<ICCPass> ICCPassRegistrator(nullptr);

/*
 * ICCPass methods
 */

int ICCPass::runAnalysisPass(IcarusPassArguments &Arguments) {
  static auto IndirectCallHandler = [&](IcarusModule &IM) {
    llvm::Module *M = IM.getModule();

    for (llvm::Function &F : *M) {
      for (auto I = llvm::inst_begin(F); I != llvm::inst_end(F); ++I) {
        if (llvm::CallBase *C = llvm::dyn_cast<llvm::CallBase>(&*I)) {
          if (C->isIndirectCall())
            IndirectCalls[IM.getFilePath()]++;
        }
      }
    }
  };

  forEachModule(Arguments, lambda::ptr<void, IcarusModule &>(IndirectCallHandler));

  for (auto &[Path, Count] : IndirectCalls) {
    llvm::errs() << Path << ": " << Count << "\n";
  }

  return 0;
}

}