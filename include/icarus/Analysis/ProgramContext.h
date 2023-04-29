//
// Created by croemheld on 20.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_ANALYSIS_PROGRAMCONTEXT_H
#define ICARUS_INCLUDE_ICARUS_ANALYSIS_PROGRAMCONTEXT_H

#include <icarus/Analysis/EngineValue.h>
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
template <typename AnalysisIterator> class ProgramContext {

  /* Map of names and addresses to global values in LLVM IR */
  std::map<std::string, llvm::Value *> NamedValues;
  std::map<uint64_t, llvm::Value *> AddressableValues;

  /* Map of global regions in memory, uniquely identifiable */
  std::map<llvm::Value *, EngineValue> EngineValues;

  std::deque<FunctionContext<AnalysisIterator>> FCStack;

public:
  ProgramContext() = default;

  bool isStackEmpty() const { return FCStack.empty(); }

  FunctionContext<AnalysisIterator> &getCurrentFunctionStack() { return FCStack.front().get(); }
};

} // namespace icarus

#endif // ICARUS_INCLUDE_ICARUS_ANALYSIS_PROGRAMCONTEXT_H
