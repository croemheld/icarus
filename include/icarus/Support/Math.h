//
// Created by croemheld on 12.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_SUPPORT_MATH_H
#define ICARUS_INCLUDE_ICARUS_SUPPORT_MATH_H

namespace icarus {

/**
 * Helper method for determining the number of digits for a given integer.
 * @tparam T The deduced type of the integer passed to this method.
 * @param Num The value of the integer for which to determine the number of digits.
 * @param Base The base for which the number of digits should be determined.
 * @return The number of digits, including minus sign, if the number is negative.
 */
template <typename T> unsigned numDigits(T Num, unsigned Base = 10) {
  unsigned Digits = 0;
  if (Num < 0)
    ++Digits;
  while (Num) {
    Num /= Base;
    ++Digits;
  }
  return Digits;
}

} // namespace icarus

#endif // ICARUS_INCLUDE_ICARUS_SUPPORT_MATH_H
