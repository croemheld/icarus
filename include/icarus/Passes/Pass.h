//
// Created by croemheld on 25.12.2022.
//

#ifndef ICARUS_PASSES_PASS_H
#define ICARUS_PASSES_PASS_H

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include <icarus/Passes/PassArguments.h>

#include <icarus/Logger/Logger.h>

#include <icarus/Support/Namespaces.h>
#include <icarus/Support/ObjectRegistry.h>
#include <icarus/Support/String.h>

namespace icarus {

namespace passes {

/*
 * Passes for the icarus tool
 */

/**
 * Base class of all passes in icarus.
 *
 * Each derived subclass of Pass has to define two static constexpr member
 * variables of type std::string_view with the name OPTION and NAME, which state
 * the CLI pass option and the full name for descriptiveness, respectively.
 * An example for the Indirect Call Counter pass is listed below.
 *
 * ~~~{.cpp}
 * static constexpr std::string_view OPTION = "ICC";
 * static constexpr std::string_view NAME = "Indirect Call Counter";
 * ~~~
 */
class Pass {

protected:
  Pass() = default;

public:
  /**
   * Each pass may have different CLI arguments to check. Implement this method and
   * check if the provided CLI arguments are all valid for this specific pass.
   * @return True, if all required CLI arguments are valid.
   */
  virtual bool checkPassArguments(PassArguments &IPA) = 0;

  /**
   * Each pass has different purposes and components to execute. These should all be
   * included in this method that is called after the specific pass has been selected.
   * @return 0 on success, else any other number indicating the error status.
   */
  virtual int runAnalysisPass(PassArguments &IPA) = 0;
};

} // namespace passes

/**
 * The PassConstructor class allows is to completely separate the registry from the pass namespace so
 * that any of the classes in the pass namespace are completely decoupled from the registry and their
 * PassInfo objects.
 */
struct PassConstructor : public ObjectConstructor<passes::Pass> {
  using ObjectConstructor<passes::Pass>::ObjectConstructor;
};

/**
 * Contains information about a pass instance.
 */
class PassInfo {

  std::string const PassOption;
  std::string const PassName;
  PassConstructor Constructor;
  cl::OptionCategory *Category;

public:
  /**
   * Create a new PassInfo object for a onRegistration pass.
   * @param PassOption The CLI option to use to select this pass.
   * @param PassName The full name of this specific pass.
   * @param Constructor A pointer to the pass-generating function.
   * @param Category A pointer to the pass option category.
   */
  PassInfo(std::string_view PassOption, std::string_view PassName, PassConstructor &&Constructor,
           cl::OptionCategory *Category)
      : PassOption(PassOption), PassName(PassName), Constructor(Constructor), Category(Category) {}

  bool isGeneralCategory() const;
  std::string const &getPassOption() const;
  std::string const &getPassName() const;
  cl::OptionCategory *getCategory() const;
  PassConstructor &getConstructor() const;
};

/**
 * Specializing ObjectRegistry for icarus pass instances.
 */
struct PassRegistry : public ObjectRegistry<std::string_view, const PassInfo, PassRegistry> {
  /**
   * Return the pass for the specified pass option.
   * @param PassOption The CLI option for the pass.
   * @return The associated pass or nullptr, if it does not exist.
   */
  static passes::Pass *getPass(std::string_view PassOption);

  /**
   * Get a list of all onRegistration OptionCategory instances for icarus.
   * @param OptionCategories The vector to populate with onRegistration OptionCategory instances.
   */
  static void populateOptionCategories(std::vector<cl::OptionCategory *> &OptionCategories);
};

/**
 * With this template class, we can store custom passes in the PassRegistry.
 * In order to do that we only have to create a static variable and category
 * as illustrated in the example below.
 *
 * ~~~{.cpp}
 * static cl::OptionCategory XCategory(Category<PassTy>);
 * // Populate XCategory with pass-specific options ...
 * static RegisterPass<PassTy> X(&XCategory);
 * ~~~
 *
 * This is essentially a simple knockoff of the llvm::RegisterPass template.
 */
template <typename PassTy, typename std::enable_if_t<std::is_base_of_v<passes::Pass, PassTy>, bool> = true>
struct RegisterPass : public PassInfo {
  /**
   * Public constructor for pass registration.
   * @param Category The option category of the new pass to register.
   */
  explicit RegisterPass(cl::OptionCategory *Category)
      : PassInfo(PassTy::OPTION, PassTy::NAME, PassConstructor(constructObject<PassTy, passes::Pass>), Category) {
    PassRegistry::registerObject(PassTy::OPTION, *this);
  }
};

/**
 * Custom parser for onRegistration passes with the PassInfo object.
 */
struct PassParser : public RegistryListener<const PassInfo *, PassRegistry>, public cl::parser<const PassInfo *> {
  explicit PassParser(cl::Option &O) : cl::parser<const PassInfo *>(O) {
    PassRegistry::addRegistrationListener(this);
  }

  /**
   * Called from cl::list::done after initialization is finished. Iterate over all
   * passes that were registered before the initialization of cl::list occurred.
   */
  void initialize();

  void onRegistration(const PassInfo *PI) override;
  void apply(const PassInfo *PI) override;
  void printOptionInfo(const cl::Option &O, size_t GlobalWidth) const override;

private:
  static int OptionCompare(const PassParser::OptionInfo *VT1, const PassParser::OptionInfo *VT2);
};

/**
 * Helper template to generate constant expression for cl::OptionCategory strings
 * @tparam PassClass The Pass subclass for which the string should be generated.
 */
template <typename PassClass> struct OptionCategory {
  static constexpr std::string_view PREFIX = "Additional options for pass '";
  static constexpr std::string_view OPTBEG = "' (";
  static constexpr std::string_view OPTEND = ")";
  static constexpr auto Value = ConcatViews<PREFIX, PassClass::NAME, OPTBEG, PassClass::OPTION, OPTEND>;
};

/**
 * Returns the string generated by OptionCategory above.
 * @tparam PassClass The Pass subclass for which the string should be generated.
 */
template <typename PassClass> static constexpr std::string_view Category = OptionCategory<PassClass>::Value;

} // namespace icarus

#endif // ICARUS_PASSES_PASS_H
