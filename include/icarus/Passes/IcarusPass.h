//
// Created by croemheld on 25.12.2022.
//

#ifndef ICARUS_INCLUDE_PASSES_PASS_H
#define ICARUS_INCLUDE_PASSES_PASS_H

#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <icarus/ADT/Container.h>

#include <icarus/Logger/Logger.h>

#include <icarus/Support/Registry.h>
#include <icarus/Support/String.h>
#include <icarus/Support/Lambda.h>
#include <icarus/Support/Namespaces.h>

#include <nlohmann/json.hpp>

namespace icarus {

/*
 * Pass arguments as a class
 */

class IcarusModule {

  std::string FilePath;
  std::string FileName;
  llvm::SMDiagnostic Err;
  llvm::LLVMContext Context;
  std::unique_ptr<llvm::Module> IRModule;

public:

  IcarusModule(std::string &FilePath);

  std::string getFilePath() const;
  std::string getFileName() const;
  llvm::Module *getModule() const;

};

class IcarusPassArguments {

  using ModuleVector = std::vector<std::unique_ptr<IcarusModule>>;
  using iterator = DereferenceIterator<ModuleVector::iterator>;

  std::string FileArg;
  std::string JSONArg;
  ModuleVector Modules;

  nlohmann::json JSON;

  void insertModule(std::string &FilePath);

public:

  IcarusPassArguments(std::string &FileArg, std::string &JSONArg);

  std::string getFile() const;
  std::string getJSON() const;
  unsigned int getNumFiles() const;
  nlohmann::json getJSONObject() const;

  /**
   * Get the IcarusModule instance for the provided file name. This method does only look
   * for the file name, and not the entire path. If there are multiple files registered that
   * have the same name (even with different paths), we only return the first one found.
   * @param Name The name of the LLVM IR file to lookup.
   * @return The associated IcarusModule instance for the provided name.
   */
  IcarusModule *getModule(std::string_view Name);

  /*
   * Iterators
   */

  iterator begin();
  iterator end();

};

/*
 * Passes for the icarus tool
 */

/**
 * Base class of all passes in icarus.
 *
 * Each derived subclass of IcarusPass has to define two static constexpr member
 * variables of type std::string_view with the name OPTION and NAME, which state
 * the CLI pass option and the full name for descriptiveness, respectively.
 * An example for the Indirect Call Counter pass is listed below.
 *
 * static constexpr std::string_view OPTION = "ICC";
 * static constexpr std::string_view NAME = "Indirect Call Counter";
 */
class IcarusPass {

protected:

  IcarusPass() {};

public:

  /**
   * Each pass may have different CLI arguments to check. Implement this method and
   * check if the provided CLI arguments are all valid for this specific pass.
   * @return True, if all required CLI arguments are valid.
   */
  virtual bool checkPassArguments() { return true; }

  /**
   * Each pass has different purposes and components to execute. These should all be
   * included in this method that is called after the specific pass has been selected.
   * @return 0 on success, else any other number indicating the error status.
   */
  virtual int runAnalysisPass(IcarusPassArguments &Arguments) = 0;

};

/**
 * Helper function to call a function for every registered LLVM module.
 * @param Arguments The provided arguments in the CLI.
 * @param Callback The callback function to apply for every LLVM module.
 */
void forEachModule(IcarusPassArguments &Arguments, std::function<void(IcarusModule &)> Callback);

/*
 *
 */

using PassConstructor = IcarusPass*(*)();

/**
 * Contains information about a pass instance.
 */
class PassInfo {

  std::string_view PassOption;
  std::string_view PassName;
  PassConstructor Ctor;
  cl::OptionCategory *Category;

public:

  /**
   * Create a new PassInfo object for a onRegistration pass.
   * @param PassOption The CLI option to use to select this pass.
   * @param PassName The full name of this specific pass.
   * @param Ctor A pointer to the pass-generating function.
   * @param Category A pointer to the pass option category.
   */
  PassInfo(std::string_view PassOption,
           std::string_view PassName,
           PassConstructor Ctor,
           cl::OptionCategory *Category)
      : PassOption(PassOption), PassName(PassName), Ctor(Ctor), Category(Category) {}

  bool isGeneralCategory() const;
  std::string_view getPassOption() const;
  std::string_view getPassName() const;
  IcarusPass *getPassInstance() const;
  cl::OptionCategory *getCategory() const;

};

/**
 * Specializing ObjectRegistry for icarus pass instances.
 */
struct PassRegistry : public ObjectRegistry<std::string_view, const PassInfo, PassRegistry> {

  /**
   * PassRegistry-specific implementation overriding the virtual method.
   */
  void registerObject(const PassInfo &PI) override;

  /**
   * Helper method to directly return the requested pass instance.
   * @param PassOption The name of the pass as used in the CLI of icarus.
   * @return A new instance of the selected IcarusPass.
   */
  IcarusPass *getPassOrNull(std::string PassOption);

  /**
   * Get a list of all onRegistration OptionCategory instances for icarus.
   * @param OptionCategories The vector to populate with onRegistration OptionCategory instances.
   */
  void populateOptionCategories(std::vector<cl::OptionCategory *> &OptionCategories);
};

/**
 * With this template class, we can store custom passes in the PassRegistry.
 * In order to do that we only have to create a static variable and category
 * as illustrated in the example below.
 *
 * static cl::OptionCategory XCategory(Category<PassClass>);
 * // Populate XCategory with pass-specific options ...
 * static RegisterPass<PassClass> X(&XCategory);
 *
 * This is essentially a simple knockoff of the llvm::RegisterPass template.
 */
template <typename PassClass>
struct RegisterPass : public PassInfo {
  /**
   * Public constructor for pass registration.
   * @param PassName The name of the pass as used in the CLI of icarus.
   */
  RegisterPass(cl::OptionCategory *Category)
      : PassInfo(PassClass::OPTION, PassClass::NAME, createObj<PassClass, IcarusPass>, Category) {
    PassRegistry::getObjectRegistry()->registerObject(*this);
  }
};

/**
 * Custom parser for onRegistration passes with the PassInfo object.
 */
struct IcarusPassParser : public RegistryListener<const PassInfo *, PassRegistry>,
                          public cl::parser<const PassInfo *> {
  IcarusPassParser(cl::Option &O) : cl::parser<const PassInfo *>(O) {
    PassRegistry::getObjectRegistry()->addRegistrationListener(this);
  }

  /**
   * Called from cl::list::done after initialization is finished. Iterate over all
   * passes that were registered before the initialization of cl::list occurred.
   */
  void initialize() {
    cl::parser<const PassInfo *>::initialize();
    enumerateObjects();
  }

  void onRegistration(const PassInfo *PI) override {
    if (PI->isGeneralCategory())
      return;
    EARLY_CONF("Register pass '", PI->getPassName(), "' (", PI->getPassOption(), ")...");
    if (findOption(PI->getPassOption()) != getNumOptions()) {
      llvm::errs() << "Trying to register pass with same argument: " << PI->getPassOption() << "\n";
      llvm_unreachable(nullptr);
    }
    addLiteralOption(PI->getPassOption(), PI, PI->getPassName());
  }

  void apply(const PassInfo *PI) override {
    onRegistration(PI);
  }

  void printOptionInfo(const cl::Option &O, size_t GlobalWidth) const override {
    IcarusPassParser *IPP = const_cast<IcarusPassParser*>(this);
    llvm::array_pod_sort(IPP->Values.begin(), IPP->Values.end(), ValCompare);
    cl::parser<const PassInfo*>::printOptionInfo(O, GlobalWidth);
  }

private:

  static int ValCompare(const IcarusPassParser::OptionInfo *VT1, const IcarusPassParser::OptionInfo *VT2) {
    return VT1->Name.compare(VT2->Name);
  }

};

/**
 * Helper template to generate constant expression for cl::OptionCategory strings
 * @tparam PassClass The IcarusPass subclass for which the string should be generated.
 */
template <typename PassClass>
struct OptionCategory {
  static constexpr std::string_view TBEG = "'";
  static constexpr std::string_view TEND = "' (";
  static constexpr std::string_view QEND = ")";
  static constexpr std::string_view PREFIX = "General options for pass ";
  static constexpr auto Value = Concat<PREFIX, TBEG, PassClass::NAME, TEND, PassClass::OPTION, QEND>;
};

/**
 * Returns the string generated by OptionCategory above.
 * @tparam PassClass The IcarusPass subclass for which the string should be generated.
 */
template <typename PassClass>
static constexpr std::string_view Category = OptionCategory<PassClass>::Value;

}

#endif // ICARUS_INCLUDE_PASSES_PASS_H
