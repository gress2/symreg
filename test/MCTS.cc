#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(Stuff, WhoCares) {
  symreg::MCTS mcts{1};

  mcts.dataset().x = {{1}, {2}, {3}};
  mcts.dataset().y = {4, 5, 6}; 

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
