//
// Created by croemheld on 17.02.2023.
//

#ifndef ICARUS_ANALYSIS_ENGINEVALUE_H
#define ICARUS_ANALYSIS_ENGINEVALUE_H

#include <llvm/IR/Value.h>

#include <map>

namespace icarus {

class EngineValue;

/**
 * Class representing an arbitrary value in memory. The purpose of the class is to store and propagate
 * values an instruction may contain during the analysis of a program. It is similar to LLVMs in-house
 * llvm::GenericValue class, except that we use a pointer to a llvm::Value directly.
 */
class ValueDelegate {

  uint64_t isNode : 1;
  uint64_t Offset : 63;

  union {
    EngineValue *Node;
    llvm::Value *CVal;
  };

public:
  ValueDelegate() : isNode(false), Offset(0) {}

  ValueDelegate(EngineValue *Node, uint64_t Offset = 0) : isNode(true), Offset(Offset), Node(Node) {}

  ValueDelegate(llvm::Value *CVal) : isNode(false), Offset(0), CVal(CVal) {}

  ValueDelegate(const ValueDelegate &Other) : isNode(Other.isNode), Offset(Other.Offset) {
    if (isNodeDelegate())
      Node = Other.Node;
    else
      CVal = Other.CVal;
  }

  /**
   * Method which checks if the delegate represents a valid value. An instance that was created by the
   * default constructor is invalid since it neither represents a node or a pointer to a node. We will
   * use invalid nodes to identify values whose content is not uniquely determinable.
   * @return True, if the value is uniquely determinable.
   */
  bool isValid() const;

  bool isNodeDelegate() const;
  EngineValue *getNode() const;
  uint64_t getOffset() const;
  ValueDelegate getDelegate() const;
};

class EngineValue {

  std::map<uint64_t, ValueDelegate> Values;

  uint64_t Size;

public:
  EngineValue(unsigned Size = 0) : Size(Size) {}

  ValueDelegate getDelegate(const ValueDelegate &Delegate);
  void setDelegate(uint64_t Offset, ValueDelegate &Delegate);
};

} // namespace icarus

#endif // ICARUS_ANALYSIS_ENGINEVALUE_H
