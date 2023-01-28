//
// Created by croemheld on 18.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_PASSES_IAAPASS_H
#define ICARUS_INCLUDE_ICARUS_PASSES_IAAPASS_H

#include <icarus/Analysis/ProgramContext.h>
#include <icarus/Analysis/FunctionContext.h>

#include <icarus/Support/JSON.h>

#include <icarus/Passes/EEAPass.h>

namespace icarus {

/*
 * Constant Propagation Analysis (CPA) Pass
 */

using IAAContextRetTy = void;

/**
 * EEAContext specialization for Input-Aware Analysis used for the IAAPass class below.
 */
struct IAAContext : EEAContext<IAAContext, IAAContextRetTy> {

  static constexpr std::string_view OPTION = "IAA";
  static constexpr std::string_view NAME = "Input-Aware Analysis";

  void worker(ProgramContext& PC);

};

/**
 * Base class for input-aware analyses with the underlying abstract interpretation-based pass. It uses
 * user-provided input to generate initial program states in order to "interpret" the program.
 */
class IAAPass : public EEAPass<IAAContext> {

  InputArguments IA;

  void parseJSONArguments(PassArguments &IPA);

public:

  bool checkPassArguments(PassArguments &IPA) override;

  int runAnalysisPass(PassArguments &IPA) override;

};

}

#endif // ICARUS_INCLUDE_ICARUS_PASSES_IAAPASS_H
