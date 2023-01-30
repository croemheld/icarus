//
// Created by croemheld on 17.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_PASSES_CPAPASS_H
#define ICARUS_INCLUDE_ICARUS_PASSES_CPAPASS_H

#include <icarus/Passes/AIAPass.h>

namespace icarus {

/*
 * Constant Propagation Analysis (CPA) Pass
 */

using IAAContextRetTy = void;

/**
 * AnalysisContext specialization for constant propagation-based analyses. The AnalysisContext is part
 * of the template parameter for the CPAPass class below.
 */
struct CPAContext : public AnalysisContext<CPAContext, IAAContextRetTy> {

};

/**
 * CPAPassImpl implements the virtual methods from the Pass class. We do this here because otherwise I
 * would need to implement them in a template class (e.g. ThreadedCPAPass below) which means that they
 * would have to be implemented in the header file. It also prevents code duplications, as there would
 * only be a total of one method instead of one method per full-specialization of the template class.
 */
struct CPAPassImpl : public Pass {

  bool checkPassArguments(PassArguments& IPA) override;

  int runAnalysisPass(PassArguments& IPA) override;

};

/**
 * Base class for constant propagation-based analysis with the underlying abstract interpretation pass
 * for performing a flow- and context-sensitive analysis on a program.
 */
template <bool Threaded>
class ThreadedCPAPass : public CPAPassImpl, public ThreadedAIAPass<CPAContext, Threaded> {};

/**
 * ThreadedCPAPass class with multithreading support disabled.
 */
struct CPAPass : public ThreadedCPAPass<false> {
  static constexpr std::string_view OPTION = "CPA";
  static constexpr std::string_view NAME = "Constant Propagation Analysis";
};

/**
 * ThreadedCPAPass class with multithreading support enabled.
 */
struct CPTPass : public ThreadedCPAPass<true> {
  static constexpr std::string_view OPTION = "CPT";
  static constexpr std::string_view NAME = "Constant Propagation Analysis (Threaded)";
};

}

#endif // ICARUS_INCLUDE_ICARUS_PASSES_CPAPASS_H
