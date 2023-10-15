//
// Created by croemheld on 18.01.2023.
//

#ifndef ICARUS_PASSES_IAAPASS_H
#define ICARUS_PASSES_IAAPASS_H

#include <icarus/Analysis/FunctionContext.h>
#include <icarus/Analysis/ProgramContext.h>

#include <icarus/Support/JSON.h>

#include <icarus/Passes/EEAPass.h>

namespace icarus::passes {

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
 * Base class for input-aware analyses with the underlying abstract interpretation-based pass. It uses
 * user-provided input to generate initial program states in order to "interpret" the program.
 */
template <bool Threaded> class ThreadedIAAPass : public ThreadedEEAPass<IAAContext, Threaded, IAAContext::Iter> {

  InputArguments IA;

  /**
   * Parse the provided JSON arguments located in the JSON file. The exact structures of the supported
   * identifiers are described in the appropriate JSON functions from_json.
   * @param IPA The reference to the class with the CLI options.
   */
  void parseJSONArguments(PassArguments &IPA) {
    IcarusModule *IM = IPA.getModuleAt(0);
    nlohmann::json &JSON = IPA.getJSONObject();
    from_json(JSON, IA, IM);
  }

protected:
  bool checkJSON(PassArguments &IPA) { return !IPA.getJSON().empty(); }

public:
  int runAnalysisPass(PassArguments &IPA) override {
    parseJSONArguments(IPA);
    return 0;
  }
};

struct IAAPass : public ThreadedIAAPass<false> {

  static constexpr std::string_view OPTION = "IAA";
  static constexpr std::string_view NAME = "Input-Aware Analysis";

  bool checkPassArguments(PassArguments &IPA) override { return ThreadedIAAPass<false>::checkJSON(IPA); }

};

struct IATPass : public ThreadedIAAPass<true> {

  static constexpr std::string_view OPTION = "IAT";
  static constexpr std::string_view NAME = "Input-Aware Analysis (Threaded)";

protected:
  using ThreadedIAAPass<true>::initializeThreadPool;

public:
  bool checkPassArguments(PassArguments &IPA) override {
    if (!ThreadedIAAPass<true>::checkJSON(IPA))
      return false;
    initializeThreadPool(IPA.getNumThreads());
    return true;
  }
};

} // namespace icarus::passes

#endif // ICARUS_PASSES_IAAPASS_H
