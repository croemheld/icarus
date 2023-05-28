//
// Created by croemheld on 04.01.2023.
//

#include <icarus/Passes/CPAPass.h>

namespace icarus::passes {

/*
 * Register pass in icarus
 */

static RegisterPass<CPAPass> CPAPass(nullptr);
static RegisterPass<CPTPass> CPTPass(nullptr);

} // namespace icarus::passes