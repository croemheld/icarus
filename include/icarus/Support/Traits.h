//
// Created by croemheld on 19.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_SUPPORT_TRAITS_H
#define ICARUS_INCLUDE_ICARUS_SUPPORT_TRAITS_H

#include <type_traits>

namespace icarus {

/**
 * Helper implementation for is_template_base_of, a variation of std::is_base_of for template classes,
 * that determines whether or not a derived class is an implementation of a template base class. Using
 * this allows ensuring that we pass valid template parameters to a template class at compile time.
 * @tparam Base The template base class (without any template parameters) to check against.
 * @tparam Derived The derived class to check.
 */
template <template <typename...> class Base, typename Derived> struct is_template_base_of_impl {
  template <typename... Ts> static constexpr std::true_type eval(const Base<Ts...> *);
  static constexpr std::false_type eval(...);
  using type = decltype(eval(std::declval<Derived *>()));
};

/**
 * Helper alias for is_template_base_of_impl that directly returns the type trait.
 */
template <template <typename...> class Base, typename Derived>
using is_template_base_of = typename is_template_base_of_impl<Base, Derived>::type;

} // namespace icarus

#endif // ICARUS_INCLUDE_ICARUS_SUPPORT_TRAITS_H
