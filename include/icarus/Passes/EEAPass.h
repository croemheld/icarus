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

};

/**
 * Alias to determine whether or not a class inherits from the EEAContext above. This is necessary for
 * ensuring the EEAPass below receives a correct template argument at compile-time.
 */
template <typename EEAContextImpl>
using is_eeacontext_impl = is_template_base_of<EEAContext, EEAContextImpl>;

/**
 * Base class for pass that uses the llvm::ExecutionEngine methods for interpreting the program. It is
 * only possible to work with this pass if an appropriate EAAContextImpl type has been provided.
 * @tparam EEAContextImpl The EEAContext implementation that uses llvm::ExecutionEngine methods.
 */
template <typename EEAContextImpl, std::enable_if_t<is_eeacontext_impl<EEAContextImpl>::value, bool> = true>
struct EEAPass : AIAPass<EEAContextImpl> {

};

}

#endif // ICARUS_INCLUDE_ICARUS_PASSES_EEAPASS_H
