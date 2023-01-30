//
// Created by croemheld on 19.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_PASSES_EEAPASS_H
#define ICARUS_INCLUDE_ICARUS_PASSES_EEAPASS_H

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>

#include <icarus/Passes/AIAPass.h>

#include <icarus/Support/Traits.h>

namespace icarus {

/**
 * AnalysisContext implementation that uses the llvm::ExecutionEngine methods with llvm::GenericValues
 * to interpret the analyzed programs. The implementation of this class is a modified version of LLVMs
 * llvm::Interpreter class for which no include-able header file exists.
 * @tparam SubClass The context that implements the llvm::InstVisitor methods.
 * @tparam RetTy The return type of the individual llvm::InstVisitor methods.
 */
template <typename SubClass, typename RetTy = void>
struct EEAContext : public llvm::ExecutionEngine, public AnalysisContext<SubClass, RetTy> {

  void worker(ProgramContext& PC) {
    while (!PC.isStackEmpty()) {
      FunctionContext &FC = PC.getCurrentFunctionStack();
      llvm::Instruction &I = FC.iterateNextInstruction();
      INFO_WITH("iaa", "Interpreting: ", I);
      AnalysisContext<SubClass, RetTy>::visit(I);
    }
  }

};

/**
 * Alias to determine whether or not a class inherits from the EEAContext above. This is necessary for
 * ensuring the EEAPass below receives a correct template argument at compile-time.
 */
template <typename EEAContextImpl>
using enable_if_eeacontext = std::enable_if_t<is_template_base_of<EEAContext, EEAContextImpl>::value, bool>;

/**
 * Base class for pass that uses the llvm::ExecutionEngine methods for interpreting the program. It is
 * only possible to work with this pass if an appropriate EAAContextImpl type has been provided.
 * @tparam EEAContextImpl The EEAContext implementation that uses llvm::ExecutionEngine methods.
 */
template <typename EEAContextImpl, bool Threaded, enable_if_eeacontext<EEAContextImpl> = true>
struct EEAPass : public ThreadedAIAPass<EEAContextImpl, Threaded> {};

}

#endif // ICARUS_INCLUDE_ICARUS_PASSES_EEAPASS_H
