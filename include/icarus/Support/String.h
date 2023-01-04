//
// Created by croemheld on 27.12.2022.
//

#ifndef ICARUS_INCLUDE_ICARUS_SUPPORT_STRING_H
#define ICARUS_INCLUDE_ICARUS_SUPPORT_STRING_H

#include <array>
#include <string_view>

namespace icarus {

/**
 * Concatenate two or more strings at compile-time. This is useful for the
 * string generation in case components are scattered throughout the project.
 * @tparam Strings The strings to concatenate.
 */
template<std::string_view const &... Strings>
struct ConcatStrings {
  static constexpr auto impl() noexcept {
    constexpr std::size_t Length = (Strings.size() + ... + 0);
    std::array<char, Length + 1> Array{};
    auto append = [i = 0, &Array](auto const &S) mutable {
      for (auto C : S)
        Array[i++] = C;
    };
    (append(Strings), ...);
    Array[Length] = 0;
    return Array;
  }

  static constexpr auto Concatenated = impl();
  static constexpr std::string_view Value {Concatenated.data(), Concatenated.size() - 1};
};

/**
 * Helper template for concatenating strings at compile-time.
 * @tparam Strings The strings to concatenate.
 */
template <std::string_view const&... Strings>
static constexpr auto Concat = ConcatStrings<Strings...>::Value;

/**
 * Workaround for using strings in switch tables. This was taken directly from StackOverflow
 * and was adjusted for C++17 which uses a std::string_view argument instead of const char *.
 * https://stackoverflow.com/questions/650162/why-cant-the-switch-statement-be-applied-to-strings
 * @param S The string to hash at compile-time.
 * @return The hash of the provided string.
 */
static constexpr unsigned int StringHash(std::string_view S) noexcept {
  unsigned int Hash = 5385;

  for (const auto &E : S)
    Hash = ((Hash << 5) + Hash) + E;

  return Hash;
}

/**
 * Workaround for missing std::string::starts_with in C++17.
 * @param Str The string to check for its prefix.
 * @param Pre The prefix to search in the string.
 * @return True, if the prefix occurs at the beginning of the string.
 */
bool startsWith(std::string_view Str, std::string_view Pre);

/**
 * Workaround for missing std::string::ends_with in C++17.
 * @param Str The string to check for its suffix.
 * @param Suf The suffix to search in the string.
 * @return True, if the suffix occurs at the end of the string.
 */
bool endsWith(std::string_view Str, std::string_view Suf);

}

#endif // ICARUS_INCLUDE_ICARUS_SUPPORT_STRING_H
