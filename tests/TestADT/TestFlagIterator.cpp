//
// Created by croemheld on 27.05.2023.
//

#include <doctest.h>

#include <icarus/ADT/FlagIterator.h>

enum TestFlags {
  A = 0b00000000000000000000000000000001,
  B = 0b00000000000000000000000000000100,
  C = 0b00000000000000000000000000010000,
  D = 0b00000000000000000000000000100000,
  E = 0b00000000000000000000000001000000,
  F = 0b00000000000000000000000100000000,
  G = 0b00000000000000000001000000000000,
  H = 0b00000000000000001000000000000000,
  I = 0b00000000000000010000000000000000,
  J = 0b00000000000100000000000000000000,
  K = 0b00000001000000000000000000000000,
  L = 0b00010000000000000000000000000000,
  M = 0b00100000000000000000000000000000,
  N = 0b10000000000000000000000000000000,
};

using namespace icarus::adt;

TEST_CASE("Testing FlagIterator with empty bitmask") {
  std::size_t Count = 0;

  for (auto FlagIter = FlagIterator(0UL); *FlagIter; ++FlagIter) {
    ++Count;
  }

  CHECK(Count == 0);
}

TEST_CASE("Testing the FlagIterator") {
  std::size_t Count = 0;

  std::uint64_t Mask = A | B | C | D | E | F | G | H | I | J | K | L | M | N;
  auto FlagIter = FlagIterator(Mask);

  SUBCASE("Testing the increment operator++") {
    CHECK(*FlagIter == A);
    CHECK(*(++FlagIter) == B);
    CHECK(*(++FlagIter) == C);
    CHECK(*(++FlagIter) == D);
    CHECK(*(++FlagIter) == E);
    CHECK(*(++FlagIter) == F);
    CHECK(*(++FlagIter) == G);
    CHECK(*(++FlagIter) == H);
    CHECK(*(++FlagIter) == I);
    CHECK(*(++FlagIter) == J);
    CHECK(*(++FlagIter) == K);
    CHECK(*(++FlagIter) == L);
    CHECK(*(++FlagIter) == M);
    CHECK(*(++FlagIter) == N);
    CHECK(*(++FlagIter) == 0UL);
  }

  SUBCASE("Testing the while-loop functionality") {
    while (*FlagIter) {
      ++Count;
      ++FlagIter;
    }

    // TODO CRO: Use magic_enum
    CHECK(Count == 14);
  }

  SUBCASE("Testing the for-loop functionality") {
    for (; *FlagIter; ++FlagIter) {
      ++Count;
    }

    // TODO CRO: Use magic_enum
    CHECK(Count == 14);
  }
}