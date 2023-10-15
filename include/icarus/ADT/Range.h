//
// Created by croemheld on 12.05.2023.
//

#ifndef ICARUS_ADT_RANGE_H
#define ICARUS_ADT_RANGE_H

#include <utility>

namespace icarus::adt {

/**
 * The range class acts as a stand-in for std::ranges::range which is not available in C++17 (C++20).
 * @tparam Iterator The type of the iterator to encapsulate in the Range object.
 */
template <typename Iterator> class Range {
  Iterator begin_iterator, end_iterator;

public:
  /**
   * Create a Range object from an existing container class.
   * @tparam Container The type of the container.
   * @param C The container to create a range for.
   */
  template <typename Container> explicit Range(Container &C) : begin_iterator(C.begin()), end_iterator(C.end()) {}

  /**
   *
   * @param begin_iterator
   * @param end_iterator
   */
  Range(Iterator begin_iterator, Iterator end_iterator)
      : begin_iterator(std::move(begin_iterator)), end_iterator(std::move(end_iterator)) {}

  Iterator begin() const {
    return begin_iterator;
  }

  Iterator end() const {
    return end_iterator;
  }

  bool empty() const {
    return begin_iterator == end_iterator;
  }
};

template <class T> Range<T> make_range(T x, T y) {
  return Range<T>(std::move(x), std::move(y));
}

template <typename T> Range<T> make_range(std::pair<T, T> p) {
  return Range<T>(std::move(p.first), std::move(p.second));
}

} // namespace icarus::adt

#endif // ICARUS_ADT_RANGE_H
