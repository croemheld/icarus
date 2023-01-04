//
// Created by croemheld on 26.12.2022.
//

#ifndef ICARUS_INCLUDE_ICARUS_PASSES_ICCPASS_H
#define ICARUS_INCLUDE_ICARUS_PASSES_ICCPASS_H

#include <llvm/Support/CommandLine.h>

#include <icarus/Passes/IcarusPass.h>

#include <icarus/Support/Namespaces.h>

namespace icarus {

class ICCPass : public IcarusPass {

  std::map<std::string, unsigned long> IndirectCalls;

public:

  static constexpr std::string_view OPTION = "ICC";
  static constexpr std::string_view NAME = "Indirect Call Counter";

  ICCPass() : IcarusPass() {}

  int runAnalysisPass(IcarusPassArguments &Arguments) override;

};

}

#endif // ICARUS_INCLUDE_ICARUS_PASSES_ICCPASS_H
