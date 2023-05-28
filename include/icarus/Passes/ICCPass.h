//
// Created by croemheld on 26.12.2022.
//

#ifndef ICARUS_PASSES_ICCPASS_H
#define ICARUS_PASSES_ICCPASS_H

#include <icarus/Passes/Pass.h>

namespace icarus::passes {

class ICCPass : public Pass {

  std::map<std::string, unsigned long> IndirectCalls;

public:
  static constexpr std::string_view OPTION = "ICC";
  static constexpr std::string_view NAME = "Indirect Call Counter";

  bool checkPassArguments(PassArguments &IPA) override;

  int runAnalysisPass(PassArguments &IPA) override;
};

} // namespace icarus::passes

#endif // ICARUS_PASSES_ICCPASS_H
