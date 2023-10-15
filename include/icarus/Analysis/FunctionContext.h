//
// Created by croemheld on 20.01.2023.
//

#ifndef ICARUS_ANALYSIS_FUNCTIONCONTEXT_H
#define ICARUS_ANALYSIS_FUNCTIONCONTEXT_H

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>

#include <llvm/ExecutionEngine/GenericValue.h>

#include <icarus/Analysis/EngineValue.h>

#include <nlohmann/json.hpp>

namespace icarus {

/**
 * A class representing the current function context at any moment during the analysis. This is a copy
 * of LLVMs llvm::ExecutionState class, renamed in order to differentiate between a global and a local
 * execution context. As the original class is part of an internal LLVM header file we are required to
 * implement it ourselves.
 *
 * This class represents a local execution context (i.e. the program state for a call frame). The code
 * for a global execution context is located in icarus::ProgramContext.
 */
template <typename AnalysisIterator> class FunctionContext {

  llvm::BasicBlock *BB;
  typename AnalysisIterator::Iter II;
  llvm::CallInst *Caller;
  std::map<llvm::Value *, ValueDelegate> Values;

protected:
  /**
   * Create a new program state for a function context
   */
  FunctionContext() : BB(nullptr), II(nullptr), Caller(nullptr) {}

public:
  llvm::Instruction *nextInstruction() {
    return *II++;
  }
};

} // namespace icarus

#endif // ICARUS_ANALYSIS_FUNCTIONCONTEXT_H
