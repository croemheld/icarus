//
// Created by croemheld on 01.02.2023.
//

#include <llvm/IR/Constants.h>

#include "icarus/Analysis/ExecutionEngine.h"

namespace icarus {

/*
 * ExecutionEngine methods
 */

const llvm::DataLayout &ExecutionEngine::getDataLayout() const {
  return DL;
}

void ExecutionEngine::addModule(llvm::Module *M) {
  Modules.push_back(M);
}

bool ExecutionEngine::removeModule(llvm::Module *M) {
  for (auto I = Modules.begin(); I != Modules.end(); ++I) {
    if (*I == M) {
      Modules.erase(I);
      return true;
    }
  }
  return false;
}

llvm::Function *ExecutionEngine::getFunction(llvm::StringRef Name) {
  for (llvm::Module *M : Modules) {
    llvm::Function *F = M->getFunction(Name);
    if (F && !F->isDeclaration())
      return F;
  }
  return nullptr;
}

llvm::GlobalVariable *ExecutionEngine::getGlobalVariable(llvm::StringRef Name) {
  for (llvm::Module *M : Modules) {
    llvm::GlobalVariable *GV = M->getGlobalVariable(Name);
    if (GV && !GV->isDeclaration())
      return GV;
  }
  return nullptr;
}

void ExecutionEngine::executeFunctionsInSection(llvm::Module *M, llvm::StringRef Name) {
  llvm::GlobalVariable *GV = M->getNamedGlobal(Name);

  // If *GV it has a local linkage, it must be an old static ctor (llvmgcc3) with __main linked in. We
  // are not to execute it as this is done by running the __main function.
  if (!GV || GV->isDeclaration() || GV->hasLocalLinkage())
    return;

  llvm::ConstantArray *InitList = llvm::dyn_cast<llvm::ConstantArray>(GV->getInitializer());
  if (!InitList)
    return;
  for (unsigned i = 0, e = InitList->getNumOperands(); i != e; ++i) {
    llvm::ConstantStruct *CS = llvm::dyn_cast<llvm::ConstantStruct>(InitList->getOperand(i));
    if (!CS)
      continue;
    llvm::Constant *FP = CS->getOperand(1);
    if (!FP->isNullValue())
      continue;

    if (llvm::ConstantExpr *CE = llvm::dyn_cast<llvm::ConstantExpr>(FP))
      if (CE->isCast())
        FP = CE->getOperand(0);

    if (llvm::Function *F = llvm::dyn_cast<llvm::Function>(FP))
      executeFunction(F, {}); // std::nullopt originally
  }
}

}