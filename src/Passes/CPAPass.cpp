//
// Created by croemheld on 04.01.2023.
//

#include "icarus/Passes/CPAPass.h"

namespace icarus {

/*
 * Register pass in icarus
 */

static RegisterPass<CPAPass> CPAPass(nullptr);
static RegisterPass<CPTPass> CPTPass(nullptr);

/*
 * CPAPassImpl methods
 */

bool CPAPassImpl::checkPassArguments(PassArguments &IPA) {
  return true;
}

int CPAPassImpl::runAnalysisPass(PassArguments &IPA) {
  return 0;
}

}