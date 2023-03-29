//
// Created by croemheld on 28.01.2023.
//

#include <llvm/AsmParser/Parser.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>

#include <icarus/Passes/PassArguments.h>

#include <icarus/Support/LLVMTypes.h>
#include <icarus/Support/LLVMValue.h>
#include <icarus/Support/JSON.h>
#include <icarus/Support/String.h>
#include <icarus/Support/Clang.h>

#include <nlohmann/json.hpp>

#include <fstream>

namespace icarus {

namespace adl_json {

void from_json(const nlohmann::json &JSON, VariableDef &VD, IcarusModule *IM) {
  VD.CVal = IM->parseConstant(JSON);
}

void from_json(const nlohmann::json &JSON, CallContext &CC, IcarusModule *IM) {
  CC.Func = IM->parseFunction(JSON.at("func").get<std::string>());

  const nlohmann::json &Args = JSON.at("args");
  for (auto &[Key, Val] : Args.items()) {
#if ICARUS_CLANG_VERSION < 10
    unsigned Arg = getAsInteger(Key);
    assert((Arg >= CC.Func->arg_size()) && "Arg out of bounds");
    llvm::Value *Argument = CC.Func->arg_begin() + Arg;
#else
    llvm::Value *Argument = CC.Func->getArg(getAsInteger(Key));
#endif
    CC.Args[Argument] = IM->parseConstant(Val);
  }
}

void from_json(const nlohmann::json &JSON, InputArguments &IA, IcarusModule *IM) {
  if (JSON.contains("variables"))
    from_json(JSON.at("variables"), IA.Variables, IM);

  if (JSON.contains("functions"))
    from_json(JSON.at("functions"), IA.Functions, IM);

  /*
   * We don't need a custom from_json method for std::vector<std::string> as this is already supported
   * by the nlohmann::json library, and it does not require additional arguments for the conversion.
   */
  if (JSON.contains("simulated"))
    JSON.at("simulated").get_to(IA.Simulated);
}

}

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

llvm::SMDiagnostic &IcarusModule::getDiagnostics() {
  return Err;
}

llvm::Type *IcarusModule::parseType(const nlohmann::json &JSON) {
  llvm::Type *T;

  /* First, try to parse type directly */
  std::string Asm = JSON.get<std::string>();
  T = llvm::parseType(Asm, Err, *IRModule);
  if (!T) {
    /* Secondly, find a struct by name */
#if ICARUS_CLANG_VERSION > 11
    T = llvm::StructType::getTypeByName(IRModule->getContext(), Asm);
#else
    T = IRModule->getTypeByName(Asm);
#endif
  }

  return T;
}

llvm::Constant *IcarusModule::parseConstant(std::string Asm) {
  return llvm::parseConstantValue(Asm, Err, *IRModule);
}

llvm::Constant *IcarusModule::parseConstant(const nlohmann::json &JSON, llvm::Type *T) {

  /*
   * We currently support three types of aggregate elements: arrays, structs, and C-style strings that
   * are essentially arrays, but with a few special rules in JSON to consider. More types like pointer
   * types or vector types might also be supported in the near future.
   */

  if (JSON.is_string()) {
    return parseConstant(JSON.get<std::string>());

    /* Add support for strings with "char" and "null" */
  } else if (JSON.contains("char") && JSON.contains("null")) {
    const nlohmann::json &Char = JSON.at("char");
    const nlohmann::json &Null = JSON.at("null");
    return llvm::ConstantDataArray::getString(IRModule->getContext(), Char.get<std::string>(), Null.get<bool>());

    /* Add support for arrays and structs with "elem" */
  } else if (JSON.contains("elem")) {
    unsigned long NumElements = getNumElements(T);
    std::vector<llvm::Constant *> Elements(NumElements);
    const nlohmann::json &Elem = JSON.at("elem");
    for (unsigned long N = 0; N < NumElements; ++N) {
      std::string S = to_string(N);
      llvm::Type *E = getElementTypeAt(T, N);
      if (Elem.contains(S)) {
        Elements[N] = parseConstant(Elem.at(S));
      } else {
        Elements[N] = llvm::UndefValue::get(E);
      }
    }
    return getConstant(T, Elements);
  }

  return nullptr;
}

llvm::Constant *IcarusModule::parseConstant(const nlohmann::json &JSON) {
  if (JSON.contains("type"))
    return parseConstant(JSON, parseType(JSON.at("type").get<std::string>()));
  return parseConstant(JSON, nullptr);
}

llvm::Function *IcarusModule::parseFunction(const nlohmann::json &JSON) {
  return IRModule->getFunction(JSON.get<std::string>());
}

/*
 * PassArguments methods
 */

PassArguments::PassArguments(std::string &FileArg, std::string &JSONArg, unsigned NumThreads)
    : FileArg(FileArg)
    , JSONArg(JSONArg)
    , NumThreads(NumThreads) {
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
  } else if (endsWith(FileArg, ".txt")) {
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

void PassArguments::insertModule(std::string &FilePath) {
  if (auto M = std::make_unique<IcarusModule>(FilePath)) {
    Modules.push_back(std::move(M));
  }
}

std::string PassArguments::getFile() const {
  return FileArg;
}

std::string PassArguments::getJSON() const {
  return JSONArg;
}

unsigned PassArguments::getNumThreads() const {
  return NumThreads;
}

unsigned int PassArguments::getNumFiles() const {
  return Modules.size();
}

nlohmann::json &PassArguments::getJSONObject() {
  return JSON;
}

IcarusModule *PassArguments::getModule(std::string_view Name) {
  for (auto const &M : Modules)
    if (M->getFileName() == Name)
      return M.get();
  return nullptr;
}

IcarusModule *PassArguments::getModuleAt(unsigned N) {
  return Modules.at(N).get();
}

PassArguments::iterator PassArguments::begin() {
  return deref_iterator(Modules.begin());
}

PassArguments::iterator PassArguments::end() {
  return deref_iterator(Modules.end());
}

}