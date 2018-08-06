#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(GetRandomInt, ReturnsIntegerWithinRange) {
  auto random = symreg::util::get_random_int(0, 10, symreg::MCTS::mt);
  ASSERT_GE(random, 0);
  ASSERT_LE(random, 10);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
