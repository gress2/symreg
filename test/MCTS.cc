#include <iostream>

#include "MCTS/MCTS.hpp"
#include "gtest/gtest.h"

TEST(Stuff, WhoCares) {
  symreg::MCTS mcts;
  mcts.symbol_table().insert({"x", -8});
  mcts.symbol_table().insert({"y", 10});
  mcts.symbol_table().insert({"z", 5});
  mcts.iterate(7);
  std::cout << mcts.to_gv() << std::endl;
  std::cout << mcts.build_result()->to_string() << std::endl;
  std::cout << mcts.build_result()->to_gv() << std::endl;
  
  ASSERT_TRUE(true);
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
