//
// Created by croemheld on 01.02.2023.
//

#ifndef ICARUS_ANALYSIS_EXECUTIONENGINE_H
#define ICARUS_ANALYSIS_EXECUTIONENGINE_H

#include <llvm/ADT/SmallVector.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/ErrorHandling.h>

#include <icarus/Analysis/EngineValue.h>

namespace icarus {

class ExecutionEngine {

  const llvm::DataLayout DL;

protected:
  // List of modules we are going to analyze. This differs from the original implementation in that we
  // do not transfer ownership to this class whenever we want to analyze a module.
  llvm::SmallVector<llvm::Module *, 1> Modules;

  /**
   * Create a new ExecutionEngine from a llvm::DataLayout instance. The instance is copied so that the
   * llvm::Module associated with it does not need to be held in order to use this class.
   * @param DL The llvm::DataLayout reference which to copy into this class.
   */
  ExecutionEngine(const llvm::DataLayout &DL) : DL(DL) {}

public:
  const llvm::DataLayout &getDataLayout() const;

  /**
   * Add a pointer to a llvm::Module for function and variable lookups. This differs from the original
   * implementation that it does not transfer ownership to this class, just as described above.
   * @param M The pointer to the llvm::Module instance to add.
   */
  virtual void addModule(llvm::Module *M);

  /**
   * Removes a llvm::Module from the list of lookups. After the module is removed from the list, every
   * variable and function that were defined in that llvm::Module cannot be looked up anymore.
   * @param M The pointer to the llvm::Module instance to remove.
   * @return True, if the llvm::Module was contained in the list.
   */
  virtual bool removeModule(llvm::Module *M);

  /**
   * Find a function by its name. This method iterates over all known llvm::Module instances and looks
   * for the function that is explicitly defined (no declarations).
   * @param Name The name of the function to look up.
   * @return A pointer to the llvm::Function instance that represents the requested function.
   */
  virtual llvm::Function *getFunction(llvm::StringRef Name);

  /**
   * Find a GlobalVariable by its name. This method iterates over all known llvm::Module instances and
   * checks, if any llvm::Module instance contains a definition (no declaration) of the variable.
   * @param Name The name of the global variable to look up.
   * @return A pointer to the llvm::GlobalVariable instance matching the provided name.
   */
  virtual llvm::GlobalVariable *getGlobalVariable(llvm::StringRef Name);

  /**
   * Execute the provided function with the specified arguments. This method returns an instance of an
   * arbitrary ValueDelegate depending on the return type.
   * @param F A pointer to the llvm::Function to execute.
   * @param ArgValues An array of llvm::GenericValue instances for the function arguments.
   * @return A ValueDelegate representing the return value of this function.
   */
  virtual ValueDelegate executeFunction(llvm::Function *F, llvm::ArrayRef<ValueDelegate> ArgValues) = 0;

  /**
   * Returns a pointer to a function. This method is mainly used in the LLVM ExecutionEngine class for
   * returning the address of library functions, not functions that are code-generated.
   * @param Name The name of the function to look up.
   * @param AbortOnFailure Indicates, whether to print an error message and abort or return nullptr.
   * @return A pointer to the specified library function.
   */
  virtual void *getPointerToNamedFunction(llvm::StringRef Name, bool AbortOnFailure = true) = 0;

  /**
   * Execute all static constructors or destructors of the specified llvm::Module instance. This needs
   * to be done before we execute the program as this is equivalent to pre-initialized variables.
   * @param M The pointer to the llvm::Module instance which to initialize or finish.
   * @param Name The name of the symbol to execute.
   */
  void executeFunctionsInSection(llvm::Module *M, llvm::StringRef Name);

  /*
   * Helper inline methods for executing global constructors and destructors.
   */

  inline void executeCtors(llvm::Module *M) {
    executeFunctionsInSection(M, "llvm.global_ctors");
  }

  inline void executeDtors(llvm::Module *M) {
    executeFunctionsInSection(M, "llvm.global_dtors");
  }
};

} // namespace icarus

#endif // ICARUS_ANALYSIS_EXECUTIONENGINE_H
