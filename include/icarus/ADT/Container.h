//
// Created by croemheld on 30.12.2022.
//

#ifndef ICARUS_INCLUDE_ICARUS_ADT_CONTAINER_H
#define ICARUS_INCLUDE_ICARUS_ADT_CONTAINER_H

#include <algorithm>
#include <memory>

namespace icarus {

/*
 * Reference utility methods
 */

/**
 * Helper method to convert a container of elements to a container holding references to each element.
 * @tparam Container The type of the original container holding the elements.
 * @tparam T The type of the elements stored in the container.
 * @tparam Allocator The standard element allocator.
 * @param C The container to transform into a container of references.
 * @return A container with the wrapped references to the original elements.
 */
template <template <typename, typename> class Container, typename T, typename Allocator = std::allocator<T>>
auto toReferences(const Container<T, Allocator> &C) {
  Container<std::reference_wrapper<typename T::element_type>, Allocator> Refs;
  std::transform(C.begin(), C.end(), std::back_inserter(Refs), [](auto &E) {
    return std::ref(*E);
  });
  return Refs;
}

/**
 * Specialization of the toReferences method for std::vector containers.
 * @tparam T The type of the elements stored in the container.
 * @tparam Allocator The standard element allocator.
 * @param V The std::vector to transform into a vector of references.
 * @return A std::vector with the wrapped references to the original elements.
 */
template <typename T, typename Allocator = std::allocator<T>>
auto toReferences(const std::vector<T, Allocator> &V) {
  std::vector<std::reference_wrapper<typename T::element_type>, Allocator> Refs;
  Refs.reserve(V.size());
  std::transform(V.begin(), V.end(), std::back_inserter(Refs), [](auto &E) {
    return std::ref(*E);
  });
  return Refs;
}

/**
 * Special iterator used to iterate over dereference-able elements.
 * @tparam BaseIterator The original iterator type of the container.
 */
template <class BaseIterator>
struct DereferenceIterator : public BaseIterator {
  using value_type = typename BaseIterator::value_type::element_type;

  explicit DereferenceIterator(const BaseIterator &Other) : BaseIterator(Other) {}

  value_type &operator*() const {
    return *(BaseIterator::operator*());
  }

  value_type *operator->() const {
    return BaseIterator::operator*().get();
  }

  value_type &operator[](size_t N) const {
    return *(BaseIterator::operator[](N));
  }

};

/**
 * Helper method to return a DereferenceIterator instance for an iterator.
 * @tparam BaseIterator The original iterator type of the container.
 * @param I The instance of the iterator to convert.
 * @return A dereference-able iterator instance for the original iterator.
 */
template <typename BaseIterator>
DereferenceIterator<BaseIterator> deref_iterator(BaseIterator I) {
  return DereferenceIterator<BaseIterator>(I);
}

}

#endif // ICARUS_INCLUDE_ICARUS_ADT_CONTAINER_H
