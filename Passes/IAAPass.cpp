//
// Created by croemheld on 18.01.2023.
//

#include <icarus/Passes/IAAPass.h>

#include <icarus/Support/JSON.h>

namespace icarus {

namespace adl_json {

void from_json(const nlohmann::json &JSON, VariableDef &VD, llvm::Module *M) {

}

void from_json(const nlohmann::json &JSON, CallContext &CC, llvm::Module *M) {

}

void from_json(const nlohmann::json &JSON, InputArguments &IA, llvm::Module *M) {
  if (JSON.contains("variables"))
    from_json(JSON.at("variables"), IA.Variables, M);

  if (JSON.contains("functions"))
    from_json(JSON.at("functions"), IA.Functions, M);

  /*
   * We don't need a custom from_json method for std::vector<std::string> as this is already supported
   * by the nlohmann::json library, and it does not require additional arguments for the conversion.
   */
  if (JSON.contains("simulated"))
    JSON.at("simulated").get_to(IA.Simulated);
}

}

/*
 * Register pass in icarus
 */

static RegisterPass<IAAPass> IAAPass(nullptr);

/*
 * IAAContext methods
 */

void IAAContext::worker(ProgramContext& PC) {
  while (!PC.isStackEmpty()) {
    FunctionContext &FC = PC.getCurrentFunctionStack();
    llvm::Instruction &I = FC.iterateNextInstruction();
    INFO_WITH("iaa", "Interpreting: ", I);
    visit(I);
  }
}

/*
 * AIAPass methods
 */

void IAAPass::parseJSONArguments(IcarusPassArguments &IPA) {

  IcarusModule *IM = IPA.getModuleAt(0);
  nlohmann::json& JSON = IPA.getJSONObject();
  from_json(JSON, IA, IM->getModule());
}

bool IAAPass::checkPassArguments(IcarusPassArguments &IPA) {
  std::string JSON = IPA.getJSON();

  /*
   * In order for the Input-Aware Analysis pass to work, we have to provide a JSON file which contains
   * the input arguments for our program's entry points. If the analysis should continue without input
   * arguments, we need to indicate this in the JSON file (e.g. by leaving out all input arguments).
   */

  if (JSON.empty())
    return false;

  return true;
}

int IAAPass::runAnalysisPass(IcarusPassArguments &IPA) {
  parseJSONArguments(IPA);
  return 0;
}

}