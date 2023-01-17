//
// Created by croemheld on 04.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_PASSES_AIAPASS_H
#define ICARUS_INCLUDE_ICARUS_PASSES_AIAPASS_H

#include <llvm/IR/InstVisitor.h>

#include <icarus/Passes/IcarusPass.h>

namespace icarus {

/**
 * Base template class for all passes that wish to use the abstract interpretation-based analyses. The
 * implementation directly controls how instructions are handled in the underlying pass.
 * @tparam SubClass The subclass that specializes this template.
 * @tparam RetTy The return type of each instruction handler. By default it is a void return type.
 */
template <typename SubClass, typename RetTy = void>
struct AnalysisContext : public llvm::InstVisitor<SubClass, RetTy> {


protected:

  /**
   * Creates a new AnalysisContext instance that inherits all methods from the llvm::InstVisitor class
   * template. The class is going to accept either a single llvm::BasicBlock or a llvm::Function which
   * contains an entry basic block for the analysis.
   */
  AnalysisContext() = default;

};

/*
 * Abstract Interpretation Analysis (AIA) Pass
 */

/**
 * Base class for all abstract interpretation-based analyses. The core logic of this class is provided
 * by the AIAContext (AnalysisContext) subclass passed as the template parameter.
 * @tparam AIAContext The AnalysisContext subclass which implements the core algorithm.
 */
template <typename AIAContext>
class AIAPass : public IcarusPass {

public:

  static constexpr std::string_view OPTION = AIAContext::OPTION;
  static constexpr std::string_view NAME = AIAContext::NAME;

  /**
   * Creates a new abstract interpretation-based analysis pass with the provided AnalysisContext. With
   * the AnalysisContext template argument, we implement the basic techniques for the pass.
   */
  AIAPass() : IcarusPass() {}

};

}

#endif // ICARUS_INCLUDE_ICARUS_PASSES_AIAPASS_H
