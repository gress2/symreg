#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

using action_factory = symreg::MCTS::simulator::action_factory;

TEST(GetSet, BasicSanityCheck) {
  std::random_device rd;
  std::mt19937 mt(rd());
  action_factory af(3, mt); 

  auto actions = af.get_set(2);
  ASSERT_TRUE(actions.size());
  ASSERT_TRUE(actions[0].get());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
