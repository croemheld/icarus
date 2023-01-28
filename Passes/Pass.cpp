//
// Created by croemheld on 12/25/22.
//

#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/Type.h>

#include <icarus/Passes/Pass.h>

#include <fstream>

namespace icarus {

/*
 * Pass helper methods
 */

void forEachModule(PassArguments &Arguments, const std::function<void(IcarusModule &)>& Callback) {
  for (IcarusModule &IM : Arguments) {
    Callback(IM);
  }
}

/*
 * PassInfo methods
 */

bool PassInfo::isGeneralCategory() const {
  return PassName.empty();
}

std::string_view PassInfo::getPassOption() const {
  return PassOption;
}

std::string_view PassInfo::getPassName() const {
  return PassName;
}

Pass *PassInfo::getPassInstance() const {
  return Ctor();
}

cl::OptionCategory *PassInfo::getCategory() const {
  return Category;
}

/*
 * PassRegistry methods
 */

void PassRegistry::registerObject(const PassInfo &PI) {
  if (!ObjectMap.count(PI.getPassOption())) {
    ObjectMap.insert(std::make_pair(PI.getPassOption(), &PI));

    for (auto *L : Listeners)
      L->onRegistration(&PI);
  }
}

Pass *PassRegistry::getPassOrNull(const std::string& PassOption) {
  if (const PassInfo *PD = this->getObjectOrNull(PassOption))
    return PD->getPassInstance();
  return nullptr;
}

void PassRegistry::populateOptionCategories(std::vector<cl::OptionCategory *> &OptionCategories) {
  for (auto &[_, PI] : ObjectMap)
    if (PI) OptionCategories.push_back(PI->getCategory());
}

/*
 * IcarusPassParser methods
 */

void IcarusPassParser::initialize() {
  cl::parser<const PassInfo *>::initialize();
  enumerateObjects();
}

void IcarusPassParser::onRegistration(const PassInfo *PI) {
  if (PI->isGeneralCategory())
    return;
  EARLY_CONF("Register pass '", PI->getPassName(), "' (", PI->getPassOption(), ")...");
  if (findOption(PI->getPassOption()) != getNumOptions()) {
    llvm::errs() << "Trying to register pass with same argument: " << PI->getPassOption() << "\n";
    llvm_unreachable(nullptr);
  }
  addLiteralOption(PI->getPassOption(), PI, PI->getPassName());
}

void IcarusPassParser::apply(const PassInfo *PI) {
  onRegistration(PI);
}

void IcarusPassParser::printOptionInfo(const cl::Option &O, size_t GlobalWidth) const {
  auto *IPP = const_cast<IcarusPassParser*>(this);
  llvm::array_pod_sort(IPP->Values.begin(), IPP->Values.end(), ValCompare);
  cl::parser<const PassInfo*>::printOptionInfo(O, GlobalWidth);
}

int IcarusPassParser::ValCompare(const IcarusPassParser::OptionInfo *VT1, const IcarusPassParser::OptionInfo *VT2) {
  return VT1->Name.compare(VT2->Name);
}

}