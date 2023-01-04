//
// Created by croemheld on 12/25/22.
//

#include <llvm/IRReader/IRReader.h>

#include <icarus/Passes/IcarusPass.h>
#include <icarus/Support/String.h>

#include <nlohmann/json.hpp>

#include <fstream>

namespace icarus {

/*
 * IcarusModule methods
 */

IcarusModule::IcarusModule(std::string &FilePath) : FilePath(FilePath) {
    FileName = llvm::StringRef(FilePath).rsplit('/').second;
    IRModule = llvm::parseIRFile(FilePath, Err, Context);
}

std::string IcarusModule::getFilePath() const {
  return FilePath;
}

std::string IcarusModule::getFileName() const {
  return FileName;
}

llvm::Module *IcarusModule::getModule() const {
  return IRModule.get();
}

/*
 * IcarusPassArguments methods
 */

IcarusPassArguments::IcarusPassArguments(std::string &FileArg, std::string &JSONArg)
    : FileArg(FileArg), JSONArg(JSONArg) {
  /*
   * First, check if the input files exists and parse all potential LLVM modules
   * in the object instance. If no input files were found, icarus will exit here.
   */
  if (!fs::exists(FileArg))
    return;

  /*
   * We support three file types: LLVM files (.bc and .ll) and plain text files.
   * The plain text files contain the locations of one or more LLVM files. These
   * can then be used to perform analyses that require two or more files.
   */
  if (endsWith(FileArg, ".bc") || endsWith(FileArg, ".ll")) {
    insertModule(FileArg);
  } else if (endsWith(FileArg, ".txt") ) {
    std::ifstream InputFile(FileArg);
    std::string FileLine;
    while (std::getline(InputFile, FileLine)) {
      insertModule(FileLine);
    }
    InputFile.close();
  }

  Modules.shrink_to_fit();

  /*
   * Secondly, we check if a JSON file has been provided. If there is one, we try
   * to parse it using nlohmann::json. If there is no JSON file, we continue. But
   * if there is a JSON file and an error occurred, we exit icarus here.
   */
  if (JSONArg.empty())
    return;
  else {
    if (!fs::exists(JSONArg)) {
      Modules.clear();
      return;
    }
  }

  std::ifstream JSONStream(JSONArg);
  JSONStream >> JSON;
  JSONStream.close();
}

void IcarusPassArguments::insertModule(std::string &FilePath) {
  if (auto M = std::make_unique<IcarusModule>(FilePath) ) {
    Modules.push_back(std::move(M));
  }
}

std::string IcarusPassArguments::getFile() const {
  return FileArg;
}

std::string IcarusPassArguments::getJSON() const {
  return JSONArg;
}

unsigned int IcarusPassArguments::getNumFiles() const {
  return Modules.size();
}

nlohmann::json IcarusPassArguments::getJSONObject() const {
  return JSON;
}

IcarusModule *IcarusPassArguments::getModule(std::string_view Name) {
  for (auto const &M : Modules)
    if (M->getFileName() == Name)
      return M.get();
  return nullptr;
}

IcarusPassArguments::iterator IcarusPassArguments::begin() {
  return deref_iterator(Modules.begin());
}

IcarusPassArguments::iterator IcarusPassArguments::end() {
  return deref_iterator(Modules.end());
}

/*
 * IcarusPass helper methods
 */

void forEachModule(IcarusPassArguments &Arguments, std::function<void(IcarusModule &)> Callback) {
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

IcarusPass *PassInfo::getPassInstance() const {
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

IcarusPass *PassRegistry::getPassOrNull(std::string PassOption) {
  if (const PassInfo *PD = this->getObjectOrNull(PassOption))
    return PD->getPassInstance();
  return nullptr;
}

void PassRegistry::populateOptionCategories(std::vector<cl::OptionCategory *> &OptionCategories) {
  for (auto &[_, PI] : ObjectMap)
    if (PI) OptionCategories.push_back(PI->getCategory());
}

}