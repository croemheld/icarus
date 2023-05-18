//
// Created by croemheld on 19.01.2023.
//

#ifndef ICARUS_PASSES_EEAPASS_H
#define ICARUS_PASSES_EEAPASS_H

#include <icarus/Analysis/EngineValue.h>
#include <icarus/Analysis/ExecutionEngine.h>

#include <icarus/Passes/AIAPass.h>
#include <icarus/Passes/Pass.h>

#include <icarus/Support/Traits.h>

namespace icarus {

/**
 * AnalysisContext implementation that uses the custom ExecutionEngine class with the new ProgramValue
 * to interpret the analyzed programs.
 * @tparam SubClass The context that implements the llvm::InstVisitor methods.
 * @tparam RetTy The return type of the individual llvm::InstVisitor methods.
 */
template <typename AnalysisIterator, typename SubClass, typename RetTy = void>
class EEAContext : public ExecutionEngine, public AnalysisContext<SubClass, RetTy> {

  ProgramContext<AnalysisIterator> PC;
  ValueDelegate ExitValue;

public:
  EEAContext(ProgramContext<AnalysisIterator> &PC, const llvm::DataLayout &DL)
      : ExecutionEngine(DL)
      , AnalysisContext<SubClass, RetTy>()
      , PC(PC) {}

  /*
   * Virtual methods from ExecutionEngine
   */

  ValueDelegate executeFunction(llvm::Function *F, llvm::ArrayRef<ValueDelegate> ArgValues) override {
    return ValueDelegate();
  }

  virtual void *getPointerToNamedFunction(llvm::StringRef Name, bool AbortOnFailure = true) override { return nullptr; }

  /**
   * Prepare the current program context for a call to the specified function with the given arguments
   * stored in an array. The actual function call is handled by the main loop.
   * @param F The function to call in the next iteration.
   * @param ArgValues The arguments to pass to this function.
   */
  void callFunction(llvm::Function *F, llvm::ArrayRef<llvm::GenericValue> ArgValues) {}
};

/**
 * Alias to determine whether or not a class inherits from the EEAContext above. This is necessary for
 * ensuring the ThreadedEEAPass below receives a correct template argument at compile-time.
 */
template <typename EEAContextImpl>
using enable_if_eeacontext = std::enable_if_t<is_template_base_of<EEAContext, EEAContextImpl>::value, bool>;

/**
 * Base class for pass that uses the llvm::ExecutionEngine methods for interpreting the program. It is
 * only possible to work with this pass if an appropriate EAAContextImpl type has been provided.
 * @tparam EEAContextImpl The EEAContext implementation that uses llvm::ExecutionEngine methods.
 */

template <typename EEAContextImpl, bool Threaded, typename Iterator>
struct ThreadedEEAPass : ThreadedAIAPass<EEAContextImpl, Threaded, Iterator> {};

template <typename EEAContextImpl, typename Iterator>
struct ThreadedEEAPass<EEAContextImpl, true, Iterator> : public ThreadedAIAPass<EEAContextImpl, true, Iterator> {
  using ThreadedAIAPass<EEAContextImpl, true, Iterator>::initializeThreadPool;
};

} // namespace icarus

#endif // ICARUS_PASSES_EEAPASS_H
