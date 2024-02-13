#include <llvm/Support/CommandLine.h>

#include <icarus/Passes/Pass.h>

#include <icarus/Logger/Logger.h>

/*
 * Sole exception uf "using namespace" here in the entire project. Use the scoped namespace
 * in the other files to avoid naming conflicts with the LLVM framework.
 */
using namespace icarus;

namespace icarus {

cl::OptionCategory IcarusCategory("General options for icarus");

cl::opt<std::string> PassOpt("pass", cl::desc("Pass option to run"), cl::cat(IcarusCategory), cl::Required);

namespace passes {

cl::list<const PassInfo *, bool, PassParser> Passes(cl::desc("Choices for --pass"), cl::cat(IcarusCategory));

} // namespace passes

cl::opt<std::string> File("file", cl::desc("Bitcode (.bc, .ll), text (.txt)"), cl::cat(IcarusCategory), cl::Required);

cl::opt<std::string> JSON("json", cl::desc("Path to JSON file for pass-specific arguments"), cl::cat(IcarusCategory));

cl::opt<unsigned> Threads("threads", cl::desc("Number of threads to run in thread pool"), cl::cat(IcarusCategory));

/*
 * Usually, we would be using the internal structs from LLVM for managing the -debug and -debug-only
 * options of icarus. However, this would require developers to always have two versions of the LLVM
 * framework installed/built for switching between the Debug and Release build.
 *
 * For this reason we provide our own implementation of these options. The options are controlled in
 * defining or leaving out the NDEBUG macro during the build process of icarus (-DNDEBUG in Release,
 * leaving out the macro in Debug).
 */

cl::OptionCategory DebugCategory("Debug options for icarus");
cl::opt<bool, true> Debug("debug", cl::desc("Enable debug log"), cl::cat(DebugCategory), cl::location(llvm::DebugFlag));
cl::opt<std::string> DebugOnly("debug-only", cl::desc("Only print selective debug messages"), cl::cat(DebugCategory));
cl::opt<std::string> DebugFile("debug-file", cl::desc("File in which to save logger output"), cl::cat(DebugCategory));

} // namespace icarus

int main(int argc, char *argv[]) {
  std::vector<cl::OptionCategory *> OptionCategories{&IcarusCategory, &DebugCategory};
  PassRegistry::populateOptionCategories(OptionCategories);
  cl::HideUnrelatedOptions(OptionCategories);

  if (!cl::ParseCommandLineOptions(argc, argv) || File.empty())
    return EINVAL;

  passes::Pass *IP = PassRegistry::getPass(PassOpt.getValue());
  if (!IP)
    return EINVAL;

  PassArguments IPA(File.getValue(), JSON.getValue(), Threads.getValue());
  if (!IPA.getNumFiles())
    return ENOENT;

  if (!IP->checkPassArguments(IPA))
    return EINVAL;

  logger::initLoggerOptions(DebugOnly, DebugFile);

  ICARUS_INFO_WITH("init", "Start icarus...");

  /*
   * Maybe in the future we can do something like pass chaining where we run multiple
   * passes in succession. Not difficult to implement, but the arrangement of several
   * CLI options and JSON args might be too much...
   *
   * For now, we only execute a single pass and directly return the result.
   */

  int Ret = IP->runAnalysisPass(IPA);

  logger::waitFinished();
  ThreadPool::shutdown();

  return Ret;
}
