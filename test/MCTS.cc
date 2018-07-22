#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(ChooseMove, ChoosesCorrectNodes) {
  symreg::search_node parent(std::make_unique<brick::AST::number_node>(0));
  parent.set_n(50);
  auto child = parent.add_child(std::make_unique<brick::AST::number_node>(1));
  child->set_n(1);
  child->set_v(14);
  child = parent.add_child(std::make_unique<brick::AST::number_node>(2));
  child->set_n(300);
  child->set_v(400);
  child = parent.add_child(std::make_unique<brick::AST::number_node>(3));
  child->set_n(5);
  child->set_v(6);
  auto choice = symreg::MCTS::choose_move(&parent);
  ASSERT_TRUE(choice == &(parent.get_children()[1]));
  child->set_n(500);
  choice = symreg::MCTS::choose_move(&parent);
  ASSERT_TRUE(choice == &(parent.get_children()[2]));
}

TEST(Iterate, ResultsInValidASTs) {
  auto ds = symreg::generate_dataset([](int x) { return x; }, 5, 1, 6);
  auto mcts = symreg::MCTS::MCTS(5, 200, ds); 
  mcts.iterate();
  auto ast = mcts.get_result(); 
  ASSERT_TRUE(ast->is_full());
}

TEST(Reset, ResultsInRootOnlyState) {
  auto ds = symreg::generate_dataset([](int x) { return x; }, 5, 1, 6);
  auto mcts = symreg::MCTS::MCTS(5, 200, ds); 
  mcts.iterate();
  mcts.reset();
  auto ast = mcts.get_result();
  ASSERT_TRUE(ast->to_string() == "+");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
