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
struct IAAContext : EEAContext<DefaultAnalysisIterator, IAAContext, IAAContextRetTy> {

  using Iter = DefaultAnalysisIterator;

  IAAContext(ProgramContext<DefaultAnalysisIterator> &PC, const llvm::DataLayout &DL)
      : EEAContext<DefaultAnalysisIterator, IAAContext, IAAContextRetTy>(PC, DL) {}

};

/**
 * Similar to CPAPassImpl, we implement the virtual methods from the Pass class here to avoid code du-
 * plication and implementing the methods in header files. Is inherited from by ThreadedIAAPass.
 */
class IAAPassImpl : public Pass {

  InputArguments IA;

  /**
   * Parse the provided JSON arguments located in the JSON file. The exact structures of the supported
   * identifiers are described in the appropriate JSON functions from_json.
   * @param IPA The reference to the class with the CLI options.
   */
  void parseJSONArguments(PassArguments &IPA);

public:

  bool checkPassArguments(PassArguments &IPA) override;

  int runAnalysisPass(PassArguments &IPA) override;

};

/**
 * Base class for input-aware analyses with the underlying abstract interpretation-based pass. It uses
 * user-provided input to generate initial program states in order to "interpret" the program.
 */

struct IAAPass : public IAAPassImpl, public EEAPass<IAAContext, IAAContext::Iter> {
  static constexpr std::string_view OPTION = "IAA";
  static constexpr std::string_view NAME = "Input-Aware Analysis";
};

struct IATPass : public IAAPassImpl, public EETPass<IAAContext, IAAContext::Iter> {
  static constexpr std::string_view OPTION = "IAT";
  static constexpr std::string_view NAME = "Input-Aware Analysis (Threaded)";
};

}

#endif // ICARUS_INCLUDE_ICARUS_PASSES_IAAPASS_H
