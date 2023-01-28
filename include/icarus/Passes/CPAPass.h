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

  static constexpr std::string_view OPTION = "CPA";
  static constexpr std::string_view NAME = "Constant Propagation Analysis";

};

/**
 * Base class for constant propagation-based analysis with the underlying abstract interpretation pass
 * for performing a flow- and context-sensitive analysis on a program.
 */
class CPAPass : public AIAPass<CPAContext> {

public:

  int runAnalysisPass(PassArguments &IPA) override;

};

}

#endif // ICARUS_INCLUDE_ICARUS_PASSES_CPAPASS_H
