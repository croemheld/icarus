//
// Created by croemheld on 04.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_PASSES_AIAPASS_H
#define ICARUS_INCLUDE_ICARUS_PASSES_AIAPASS_H

#include <llvm/IR/InstVisitor.h>

#include <icarus/Analysis/ProgramContext.h>
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
using enable_if_aiacontext = std::enable_if_t<is_template_base_of<AnalysisContext, AIAContextImpl>::value, bool>;

/*
 * Abstract Interpretation Analysis (AIA) Pass
 */

/**
 * Base class for all abstract interpretation-based analyses. The core logic of this class is provided
 * by the AIAContext (AnalysisContext) subclass passed as the template parameter. In order to use this
 * class, one has to inherit from both Pass and ThreadedAIAPass.
 * @tparam AIAContextImpl The AnalysisContext subclass which implements the core algorithm.
 */
template <typename AIAContextImpl, bool Threaded, enable_if_aiacontext<AIAContextImpl> = true>
class ThreadedAIAPass {

public:

  /**
   * Creates a new abstract interpretation-based analysis pass with the provided AnalysisContext. With
   * the AnalysisContext template argument, we implement the basic techniques for the pass.
   */
  ThreadedAIAPass() = default;

  /**
   * Schedule method for ThreadedAIAPass subclasses that support multithreading.
   * @tparam UseThreadPool If UseThreadPool is true, enable this method.
   * @param PC The program context to schedule.
   */
  template <bool UseThreadPool = Threaded, typename std::enable_if_t<UseThreadPool, bool>::value = true>
  void schedule(ProgramContext& PC) {

  }

  /**
   * Schedule method for ThreadedAIAPass subclasses that do NOT support multithreading.
   * @tparam UseThreadPool If UseThreadPool is false, enable this method.
   * @param PC The program context to schedule.
   */
  template <bool UseThreadPool = Threaded, typename std::enable_if_t<!UseThreadPool, bool>::value = true>
  void schedule(ProgramContext& PC) {

  }

};

}

#endif // ICARUS_INCLUDE_ICARUS_PASSES_AIAPASS_H
