//
// Created by croemheld on 04.01.2023.
//

#include <llvm/IR/Instructions.h>

#include <icarus/Passes/IcarusPass.h>
#include <icarus/Passes/AIAPass.h>

namespace icarus {

/*
 * Register pass in icarus
 */

static RegisterPass<CPAPass> CPAPass(nullptr);

/*
 * AIAPass methods
 */

int CPAPass::runAnalysisPass(IcarusPassArguments &Arguments) {
  return 0;
}

}