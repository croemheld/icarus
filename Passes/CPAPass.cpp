//
// Created by croemheld on 04.01.2023.
//

#include <icarus/Passes/CPAPass.h>

namespace icarus {

/*
 * Register pass in icarus
 */

static RegisterPass<CPAPass> CPAPass(nullptr);

/*
 * AIAPass methods
 */

int CPAPass::runAnalysisPass(IcarusPassArguments &IPA) {
  return 0;
}

}