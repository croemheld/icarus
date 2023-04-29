//
// Created by croemheld on 18.01.2023.
//

#include <llvm/AsmParser/Parser.h>

#include "icarus/Passes/IAAPass.h"

#include "icarus/Support/JSON.h"
#include "icarus/Support/LLVMTypes.h"

namespace icarus {

/*
 * Register pass in icarus
 */

static RegisterPass<IAAPass> IAAPass(nullptr);
static RegisterPass<IATPass> IATPass(nullptr);

/*
 * IATPass methods
 */

} // namespace icarus