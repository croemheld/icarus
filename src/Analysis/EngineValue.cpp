//
// Created by croemheld on 27.02.2023.
//

#include "icarus/Analysis/EngineValue.h"

namespace icarus {

/*
 * ValueDelegate methods
 */

bool ValueDelegate::isValid() const {
  return isNode ? Node != nullptr : CVal && !Offset;
}

bool ValueDelegate::isNodeDelegate() const {
  return isValid() && isNode;
}

EngineValue *ValueDelegate::getNode() const {
  assert((isNode));
  return Node;
}

uint64_t ValueDelegate::getOffset() const {
  return Offset;
}

ValueDelegate ValueDelegate::getDelegate() const {
  return Node->getDelegate(*this);
}

/*
 * EngineValue methods
 */

ValueDelegate EngineValue::getDelegate(const ValueDelegate &Delegate) {
  assert((Delegate.isNodeDelegate() && Delegate.getNode() == this));
  return Values[Delegate.getOffset()];
}

void EngineValue::setDelegate(uint64_t Offset, ValueDelegate &Delegate) {
  Values[Offset] = Delegate;
}

}