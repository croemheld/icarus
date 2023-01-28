//
// Created by croemheld on 04.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_PASSES_AIAPASS_H
#define ICARUS_INCLUDE_ICARUS_PASSES_AIAPASS_H

#include <llvm/IR/InstVisitor.h>

#include <icarus/Passes/Pass.h>

#include <icarus/Support/Traits.h>

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


/**
 * Alias to determine whether or not a class inherits from the AIAContext above. This is necessary for
 * ensuring the AIAPass below receives a correct template argument at compile-time.
 */
template <typename AIAContextImpl>
using is_aiacontext_impl = is_template_base_of<AnalysisContext, AIAContextImpl>;

/*
 * Abstract Interpretation Analysis (AIA) Pass
 */

/**
 * Base class for all abstract interpretation-based analyses. The core logic of this class is provided
 * by the AIAContext (AnalysisContext) subclass passed as the template parameter.
 * @tparam AIAContextImpl The AnalysisContext subclass which implements the core algorithm.
 */
template <typename AIAContextImpl, std::enable_if_t<is_aiacontext_impl<AIAContextImpl>::value, bool> = true>
class AIAPass : public Pass {

public:

  static constexpr std::string_view OPTION = AIAContextImpl::OPTION;
  static constexpr std::string_view NAME = AIAContextImpl::NAME;

  /**
   * Creates a new abstract interpretation-based analysis pass with the provided AnalysisContext. With
   * the AnalysisContext template argument, we implement the basic techniques for the pass.
   */
  AIAPass() : Pass() {}

};

}

#endif // ICARUS_INCLUDE_ICARUS_PASSES_AIAPASS_H
