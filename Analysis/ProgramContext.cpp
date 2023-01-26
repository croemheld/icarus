//
// Created by croemheld on 22.01.2023.
//

#include <icarus/Analysis/ProgramContext.h>

namespace icarus {

bool ProgramContext::isStackEmpty() const {
  return FCStack.empty();
}

FunctionContext& ProgramContext::getCurrentFunctionStack() {
  return FCStack.top();
}

}