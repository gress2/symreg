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

TEST(GetSet, ArityMatchesParam) {
  std::random_device rd;
  std::mt19937 mt(rd());
  action_factory af(1, mt);

  for (int i = 0; i < 3; i++) {
    auto actions = af.get_set(i);
    for (auto& action : actions) {
      ASSERT_TRUE(action->num_children() <= i);
    }
  }
}

TEST(GetRandom, BasicSanityCheck) {
  std::random_device rd;
  std::mt19937 mt(rd());
  action_factory af(1, mt);

  auto action = af.get_random(2);
  auto str = action->to_string();
  ASSERT_TRUE(str.size());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
