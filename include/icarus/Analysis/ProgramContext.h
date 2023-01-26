//
// Created by croemheld on 20.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_ANALYSIS_PROGRAMCONTEXT_H
#define ICARUS_INCLUDE_ICARUS_ANALYSIS_PROGRAMCONTEXT_H

#include <icarus/Analysis/FunctionContext.h>

#include <stack>

namespace icarus {

/**
 * A class representing the a complete program context that includes the currently analyzed call stack
 * at any point during the analysis.
 *
 * This class represents a global execution context (i.e. the program state of a program). For a local
 * execution context, we user the class described in icarus::FunctionContext.
 */
class ProgramContext {

  std::stack<FunctionContext> FCStack;
  llvm::GenericValue ExitValue;

public:

  ProgramContext() = default;

  bool isStackEmpty() const;

  /**
   * @return A reference to the FunctionContext currently sitting on top of the stack.
   */
  FunctionContext& getCurrentFunctionStack();

};

}

#endif // ICARUS_INCLUDE_ICARUS_ANALYSIS_PROGRAMCONTEXT_H
