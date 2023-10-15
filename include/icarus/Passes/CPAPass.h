//
// Created by croemheld on 17.01.2023.
//

#ifndef ICARUS_PASSES_CPAPASS_H
#define ICARUS_PASSES_CPAPASS_H

#include <icarus/Passes/AIAPass.h>

namespace icarus::passes {

/*
 * Constant Propagation Analysis (CPA) Pass
 */

using IAAContextRetTy = void;

/**
 * AnalysisContext specialization for constant propagation-based analyses. The AnalysisContext is part
 * of the template parameter for the CPAPass class below.
 */
struct CPAContext : public AnalysisContext<DefaultAnalysisIterator, CPAContext, IAAContextRetTy> {

  using Iter = DefaultAnalysisIterator;
};

/**
 * Base class for constant propagation-based analysis with the underlying abstract interpretation pass
 * for performing a flow- and context-sensitive analysis on a program.
 */
template <bool Threaded> class ThreadedCPAPass : public ThreadedAIAPass<CPAContext, Threaded, CPAContext::Iter> {

public:
  bool checkPassArguments(PassArguments &IPA) override {
    return true;
  }

  int runAnalysisPass(PassArguments &IPA) override {
    return 0;
  }
};

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

} // namespace icarus::passes

#endif // ICARUS_PASSES_CPAPASS_H
