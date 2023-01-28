//
// Created by croemheld on 27.12.2022.
//

#ifndef ICARUS_INCLUDE_ICARUS_SUPPORT_STRING_H
#define ICARUS_INCLUDE_ICARUS_SUPPORT_STRING_H

#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>

#include <array>
#include <string_view>

namespace icarus {

namespace adl_string {

using std::to_string;

/**
 * Helper function for converting a llvm::Value instance to a string representation that can be logged
 * in our threaded logger. Needs to be declared before adl_string::as_string, so that the lookup might
 * succeed in the first place.
 */
std::string to_string(const llvm::Value& V);

/**
 * Helper function for converting a llvm::Type instance to a string representation that can be logged.
 */
std::string to_string(const llvm::Type& V);

/**
 * Core function for all adl_string::to_string functions for different argument types in order to make
 * this work, simply implement a function with the name to_string in this namespace with the arguments
 * being the reference to the instance that should be converted into a string.
 * @tparam T The type of the object to convert into a string.
 * @param t The instance that is going to be converted into a string.
 * @return A string representation based on the custom implementation provided by to_string.
 */
template <typename T>
std::string as_string(const T& t) {
  return to_string(t);
}

}

/**
 * Core function in icarus namespace for to_string. All calls to to_string go through here, before the
 * appropriate function is looked up in the adl_string namespace defined above.
 * @tparam T The type of the object to convert into a string.
 * @param t The instance that is going to be converted into a string.
 * @return A string representation based on the custom implementation provided by to_string.
 */
template <typename T>
std::string to_string(const T& t) {
  return adl_string::as_string(t);
}

/**
 * Defining operator<< for llvm::Value objects and derivatives. This is necessary for storing it in an
 * instance of std::stringstream for the threaded logger. Without it, we would not be able to print an
 * instruction or value in general.
 * @param Out The stream to which to write the llvm::Value instance as a string.
 * @param V The value to write to the stream in a string representation.
 * @return The provided stream to allow operator chaining.
 */
std::ostream& operator<<(std::ostream& Out, const llvm::Value& V);

/**
 * Defining operator<< for llvm::Value objects and derivatives.
 * @param Out The stream to which to write the llvm::Value instance as a string.
 * @param T The type to write to the stream in a string representation.
 * @return The provided stream to allow operator chaining.
 */
std::ostream& operator<<(std::ostream& Out, const llvm::Type& T);

/**
 * Helper method for generating formatted strings in printf style.
 * @tparam Args deduced The type of the arguments passed to this method.
 * @param Format The format string.
 * @param args The arbitrarily typed arguments to include in this format string.
 * @return A newly formatted string with the provided arguments.
 */
template <typename ... Args>
std::string format(const char *Format, Args&&... args) {
  int Size = std::snprintf(nullptr, 0, Format, args...);
  if (Size <= 0)
    throw std::runtime_error("Error during formatting");
  std::string Formatted(Size, char());
  std::snprintf(Formatted.data(), Size + 1, Format, args...);
  return Formatted;
}

/**
 * Concatenate two or more strings at compile-time. This is useful for the
 * string generation in case components are scattered throughout the project.
 * @tparam Strings The strings to concatenate.
 */
template <std::string_view const &... Strings>
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
