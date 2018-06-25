#include <iostream>

#include "MCTS/MCTS.hpp"
#include "gtest/gtest.h"

TEST(Stuff, WhoCares) {
  symreg::MCTS mcts;
  mcts.iterate(1);
  std::cout << mcts.to_gv() << std::endl;
}
