#include <llvm/Support/CommandLine.h>

#include <icarus/Passes/IcarusPass.h>

/*
 * Sole exception uf "using namespace" here in the entire project. Use the scoped namespace
 * in the other files to avoid naming conflicts with the LLVM framework.
 */
using namespace icarus;

static cl::OptionCategory IcarusCategory("General options for icarus");

cl::opt<std::string>
Pass("pass", cl::desc("Pass option to runAnalysisPass"), cl::cat(IcarusCategory));
static cl::alias PassAlias("p", cl::desc("Alias for --pass"), cl::aliasopt(Pass));

cl::list<const PassInfo *, bool, IcarusPassParser>
PassesList(cl::desc("Choices for --pass= (without leading --)"), cl::cat(IcarusCategory));

static cl::opt<std::string>
File("file", cl::desc("Bitcode or IR (.bc, .ll), or path file (.txt)"), cl::cat(IcarusCategory));
static cl::alias FileAlias("f", cl::desc("Alias for --file"), cl::aliasopt(File));

static cl::opt<std::string>
JSON("json", cl::desc("Path to JSONArg file for pass-specific arguments"), cl::cat(IcarusCategory));
static cl::alias JSONAlias("j", cl::desc("Alias for --json"), cl::aliasopt(JSON));

int main(int argc, char *argv[]) {
  PassRegistry *PR = PassRegistry::getObjectRegistry();

  llvm::errs() << "Start icarus...\n";

  std::vector<cl::OptionCategory *> OptionCategories { &IcarusCategory };
  PR->populateOptionCategories(OptionCategories);
  cl::HideUnrelatedOptions(OptionCategories);

  if (!cl::ParseCommandLineOptions(argc, argv) || File.empty())
    return EINVAL;

  IcarusPass *IP = PR->getPassOrNull(Pass.getValue());
  if (!IP || !IP->checkPassArguments())
    return EINVAL;

  IcarusPassArguments IPA(File.getValue(), JSON.getValue());
  if (!IPA.getNumFiles())
    return ENOENT;

  return IP->runAnalysisPass(IPA);
}
