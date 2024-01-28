//
// Created by croemheld on 12.05.2023.
//

#ifndef ICARUS_ADT_FLAGITERATOR_H
#define ICARUS_ADT_FLAGITERATOR_H

#include <icarus/ADT/Range.h>

#include <cstdint>
#include <iterator>

namespace icarus::adt {

/**
 * A helper struct for iterating over set bits (i.e., flags) in a bitmask.
 */
struct FlagIterator {

  using difference_type = int;
  using value_type = std::uint64_t;
  using pointer = int *;
  using reference = int &;
  using iterator_category = std::forward_iterator_tag;

  /**
   * Creates a new FlagIterator instance for iterating over all set bits (i.e., flags) in a bitmask.
   * @param V The bitmask to use for iterating over all active flags.
   */
  explicit FlagIterator(std::uint64_t V);

  FlagIterator(FlagIterator const &iterator) : FlagIterator(iterator.getMask()) {}

  /**
   * @return The original bitmask the iterator was initialized with.
   */
  std::uint64_t getMask() const {
    return Mask;
  }

  std::uint64_t getFlag() const {
    return Flag;
  }

  std::uint64_t getIter() const {
    return Iter;
  }

  /**
   * @return The currently active flag of the bitmask.
   */
  std::uint64_t operator*() const;

  /**
   * @return A reference to this FlagIterator instance with the next active flag.
   */
  FlagIterator &operator++();

  bool operator!=(FlagIterator const &iterator) const;

private:
  std::uint64_t Mask;
  std::uint64_t Iter;
  std::uint64_t Flag;
};

/**
 * Create a Range<FlagIterator> instance to directly iterate over all active flags in a bitmask.
 * @param Mask The bitmask to check for active flags.
 * @return A Range<FlagIterator> instance that can directly be used to iterate over all flags.
 */
adt::Range<FlagIterator> flag_range(std::uint64_t Mask);

} // namespace icarus::adt

#endif // ICARUS_ADT_FLAGITERATOR_H
