//
// Created by croemheld on 22.01.2023.
//

#include <icarus/Analysis/FunctionContext.h>

namespace icarus {

llvm::Instruction& FunctionContext::iterateNextInstruction() {
  return *II++;
}

}