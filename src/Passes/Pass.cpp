//
// Created by croemheld on 12/25/22.
//

#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/Type.h>

#include <icarus/Passes/Pass.h>

#include <fstream>

namespace icarus {

/*
 * PassInfo methods
 */

bool PassInfo::isGeneralCategory() const {
  return PassName.empty();
}

std::string const &PassInfo::getPassOption() const {
  return PassOption;
}

std::string const &PassInfo::getPassName() const {
  return PassName;
}

cl::OptionCategory *PassInfo::getCategory() const {
  return Category;
}

PassConstructor &PassInfo::getConstructor() const {
  return const_cast<PassConstructor &>(Constructor);
}

/*
 * PassRegistry methods
 */

passes::Pass *PassRegistry::getPass(std::string_view PassOption) {
  return PassRegistry::getObject(PassOption, [](auto *PI) { return PI->getConstructor()(); });
}

void PassRegistry::populateOptionCategories(std::vector<cl::OptionCategory *> &OptionCategories) {
  auto RegisteredOptions = PassRegistry::getObjects([](auto *PI) { return PI->getCategory(); });
  OptionCategories.insert(OptionCategories.end(), RegisteredOptions.begin(), RegisteredOptions.end());
}

/*
 * PassParser methods
 */

void PassParser::initialize() {
  cl::parser<const PassInfo *>::initialize();
  enumerateObjects();
}

void PassParser::onRegistration(const PassInfo *PI) {
  if (PI->isGeneralCategory())
    return;

  EARLY_CONF("Register pass '", PI->getPassName(), "' (", PI->getPassOption(), ")...");
  if (findOption(PI->getPassOption()) != getNumOptions()) {
    llvm::errs() << "Trying to register pass with same argument: " << PI->getPassOption() << "\n";
    llvm_unreachable(nullptr);
  }
  addLiteralOption(PI->getPassOption(), PI, PI->getPassName());
}

void PassParser::apply(const PassInfo *PI) {
  onRegistration(PI);
}

void PassParser::printOptionInfo(const cl::Option &O, size_t GlobalWidth) const {
  auto *IPP = const_cast<PassParser *>(this);
  llvm::array_pod_sort(IPP->Values.begin(), IPP->Values.end(), OptionCompare);

  /* Print help string for pass choices */
  llvm::outs().indent(2) << O.HelpStr << ":\n";
  for (auto &PassParserOption : IPP->Values) {
    llvm::outs().indent(6) << PassParserOption.Name << ": " << PassParserOption.HelpStr << "\n";
  }

  llvm::outs() << "\n";
}

int PassParser::OptionCompare(const PassParser::OptionInfo *VT1, const PassParser::OptionInfo *VT2) {
  return VT1->Name.compare(VT2->Name);
}

} // namespace icarus