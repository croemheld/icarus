//
// Created by croemheld on 01.01.2023.
//
#include <llvm/IR/Value.h>

#include <llvm/Support/raw_ostream.h>

#include <icarus/Support/String.h>

#include <string>

namespace icarus {

namespace adl_serializer {

std::string to_string(const llvm::Value &V) {
  std::string String;
  llvm::raw_string_ostream OS(String);

  /*
   * This will print the value into a string in exactly the same way the value is represented in ".ll"
   * files of the program's LLVM IR. In order to return only the operand of the value, use the correct
   * function getAsStringOperand below.
   */
  V.print(OS, false);

  return String;
}

}

std::ostream& operator<<(std::ostream& Out, const llvm::Value& V) {
  return Out << to_string(V);
}

bool startsWith(std::string_view Str, std::string_view Pre) {
  return Str.substr(0, Pre.size()) == Pre;
}

bool endsWith(std::string_view Str, std::string_view Suf) {
  return Str.size() >= Suf.size() && 0 == Str.compare(Str.size() - Suf.size(), Suf.size(), Suf);
}

}