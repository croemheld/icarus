//
// Created by croemheld on 28.01.2023.
//

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>

#include <icarus/Support/LLVMValue.h>

namespace icarus {

llvm::Constant *getConstant(llvm::Type *T) {
  return llvm::UndefValue::get(T);
}

llvm::Constant *getConstant(llvm::Type *T, llvm::ArrayRef<llvm::Constant *> Elements) {
  if (T->isArrayTy()) {
    return getConstant<llvm::ArrayType, llvm::ConstantArray>(T, Elements);
  } else if (T->isStructTy()) {
    return getConstant<llvm::StructType, llvm::ConstantStruct>(T, Elements);
  }
  return nullptr;
}

}