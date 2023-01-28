//
// Created by croemheld on 27.01.2023.
//

#include <llvm/IR/DerivedTypes.h>

#include <icarus/Support/LLVMTypes.h>

namespace icarus {

unsigned long getNumElements(llvm::Type *T) {
  if (T->isArrayTy())
    return T->getArrayNumElements();
  else if (T->isStructTy())
    return T->getStructNumElements();
  return 0;
}

llvm::Type *getElementTypeAt(llvm::Type *T, unsigned long N) {
  if (T->isArrayTy())
    return llvm::dyn_cast<llvm::ArrayType>(T)->getElementType();
  else if (T->isStructTy())
    return llvm::dyn_cast<llvm::StructType>(T)->getElementType(N);
  return nullptr;
}

unsigned long getAsInteger(llvm::StringRef Str, unsigned int Radix) {
  llvm::APInt Int;
  Str.getAsInteger(Radix, Int);
  return Int.getZExtValue();
}

}