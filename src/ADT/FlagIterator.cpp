//
// Created by croemheld on 12.05.2023.
//

#include "icarus/ADT/FlagIterator.h"

namespace icarus::adt {

FlagIterator::FlagIterator(std::uint64_t V) : Mask(V), Iter(V) {
  Flag = Iter & -Iter;
  Iter ^= Flag;
}

std::uint64_t FlagIterator::operator*() const {
  return Flag;
}

FlagIterator &FlagIterator::operator++() {
  Flag = Iter & -Iter;
  Iter ^= Flag;
  return *this;
}

adt::Range<FlagIterator> flag_range(std::uint64_t Mask) {
  return make_range(FlagIterator(Mask), FlagIterator(0UL));
}

} // namespace icarus::adt