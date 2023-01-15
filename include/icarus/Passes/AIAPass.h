//
// Created by croemheld on 04.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_PASSES_CPAPASS_H
#define ICARUS_INCLUDE_ICARUS_PASSES_CPAPASS_H

#include <llvm/IR/InstVisitor.h>
#include <llvm/Support/CommandLine.h>

#include <icarus/Passes/IcarusPass.h>

#include <icarus/Support/Namespaces.h>

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

/*
 * Constant Propagation Analysis (CPA) Pass
 */

using CPAContextRetTy = void;

/**
 * AnalysisContext specialization for constant propagation-based analyses. The AnalysisContext is part
 * of the template parameter for the CPAPass class below.
 */
struct CPAContext : AnalysisContext<CPAContext, CPAContextRetTy> {

  static constexpr std::string_view OPTION = "CPA";
  static constexpr std::string_view NAME = "Constant Propagation Analysis";

};

/**
 * Base class for constant propagation-based analysis with the underlying abstract interpretation pass
 * for performing a flow- and context-sensitive analysis on a program.
 */
class CPAPass : public AIAPass<CPAContext> {

public:

  CPAPass() = default;

  int runAnalysisPass(IcarusPassArguments &Arguments) override;

};

}

#endif // ICARUS_INCLUDE_ICARUS_PASSES_CPAPASS_H
