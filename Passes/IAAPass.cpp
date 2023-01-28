//
// Created by croemheld on 18.01.2023.
//

#include <llvm/AsmParser/Parser.h>

#include <icarus/Passes/IAAPass.h>

#include <icarus/Support/JSON.h>
#include <icarus/Support/LLVMTypes.h>

namespace icarus {

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

void IAAPass::parseJSONArguments(PassArguments &IPA) {

  IcarusModule *IM = IPA.getModuleAt(0);
  nlohmann::json& JSON = IPA.getJSONObject();
  from_json(JSON, IA, IM);
}

bool IAAPass::checkPassArguments(PassArguments &IPA) {
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

int IAAPass::runAnalysisPass(PassArguments &IPA) {
  parseJSONArguments(IPA);
  return 0;
}

}