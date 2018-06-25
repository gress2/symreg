#include <iostream>

#include "MCTS/MCTS.hpp"
#include "gtest/gtest.h"

TEST(Stuff, WhoCares) {
  symreg::MCTS mcts;
  mcts.iterate(3);
  std::cout << mcts.to_gv() << std::endl;
  ASSERT_TRUE(false);
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
