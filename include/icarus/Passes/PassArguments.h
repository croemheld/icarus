//
// Created by croemheld on 28.01.2023.
//

#ifndef ICARUS_PASSES_PASSARGUMENTS_H
#define ICARUS_PASSES_PASSARGUMENTS_H

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

#include <icarus/ADT/Container.h>

#include <icarus/Support/Namespaces.h>

#include <nlohmann/json.hpp>

namespace icarus {

/*
 * Input argument structure
 */

struct VariableDef {
  llvm::Constant *CVal;
};

struct CallContext {
  std::string Name;
  llvm::Function *Func;
  std::map<llvm::Value *, llvm::Constant *> Args;
};

struct InputArguments {
  std::map<std::string, VariableDef> Variables;
  std::vector<CallContext> Functions;
  std::vector<std::string> Simulated;
};

/**
 * Proxy class for the actual llvm::Module instance. It acts as a helper wrapper in order to parse the
 * JSON values, store various information about the associated bitcode file as well as dumping the new
 * (modified) bitcode into a dedicated file.
 */
class IcarusModule {

  std::string FilePath;
  std::string FileName;
  llvm::SMDiagnostic Err;
  llvm::LLVMContext Context;
  std::unique_ptr<llvm::Module> IRModule;

public:
  explicit IcarusModule(std::string &FilePath);

  std::string getFilePath() const;
  std::string getFileName() const;
  llvm::Module *getModule() const;
  llvm::SMDiagnostic &getDiagnostics();

  llvm::Type *parseType(const nlohmann::json &JSON);
  llvm::Constant *parseConstant(std::string Asm);
  llvm::Constant *parseConstant(const nlohmann::json &JSON, llvm::Type *T);
  llvm::Constant *parseConstant(const nlohmann::json &JSON);
  llvm::Function *parseFunction(const nlohmann::json &JSON);
};

/**
 * Class for managing pass arguments in icarus. The class is instantiated once in the beginning and is
 * passed to each pass' analysis function. It stores all opened llvm::Module instances in a vector.
 */
class PassArguments {

  using ModuleVector = std::vector<std::unique_ptr<IcarusModule>>;
  using iterator = DereferenceIterator<ModuleVector::iterator>;

  std::string FileArg;
  std::string JSONArg;
  unsigned NumThreads;
  ModuleVector Modules;

  nlohmann::json JSON;

  void insertModule(std::string &FilePath);

public:
  PassArguments(std::string &FileArg, std::string &JSONArg, unsigned NumThreads);

  std::string getFile() const;
  std::string getJSON() const;
  unsigned getNumThreads() const;
  unsigned int getNumFiles() const;
  nlohmann::json &getJSONObject();

  /**
   * Get the IcarusModule instance for the provided file name. This method does only look
   * for the file name, and not the entire path. If there are multiple files registered that
   * have the same name (even with different paths), we only return the first one found.
   * @param Name The name of the LLVM IR file to lookup.
   * @return The associated IcarusModule instance for the provided name.
   */
  IcarusModule *getModule(std::string_view Name);

  /**
   * Return the module at the specified index. The modules are sorted in insertion order.
   * @param N The index of the vector from which to retrieve the module.
   * @return The IcarusModule instance at index N.
   */
  IcarusModule *getModuleAt(unsigned N);

  /*
   * Iterators
   */

  iterator begin();
  iterator end();
};

} // namespace icarus

#endif // ICARUS_PASSES_PASSARGUMENTS_H
