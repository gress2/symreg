#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(ChooseMove, ChoosesCorrectNodes) {
  symreg::search_node parent(std::make_unique<brick::AST::number_node>(0));
  parent.set_n(50);
  parent.add_child(std::make_unique<brick::AST::number_node>(1));
  auto* child = &(parent.get_children().back());
  child->set_n(1);
  child->set_q(14);
  parent.add_child(std::make_unique<brick::AST::number_node>(2));
  child = &(parent.get_children().back());
  child->set_n(300);
  child->set_q(400);
  parent.add_child(std::make_unique<brick::AST::number_node>(3));
  child = &(parent.get_children().back());
  child->set_n(5);
  child->set_q(6);
  auto choice = symreg::MCTS::choose_move(&parent, 0);
  ASSERT_TRUE(choice == &(parent.get_children()[1]));
  child->set_n(500);
  choice = symreg::MCTS::choose_move(&parent, 0);
  ASSERT_TRUE(choice == &(parent.get_children()[2]));
}

TEST(Iterate, ResultsInValidASTs) {
  auto mab = std::make_shared<symreg::MCTS::scorer::UCB1>();
  auto ds = symreg::generate_dataset([](int x) { return x; }, 5, 1, 6);
  auto loss = std::make_shared<symreg::loss_fn::NRMSD>();
  auto lp = std::make_shared<symreg::MCTS::simulator::leaf_picker::recursive_random_child_picker>();
  symreg::MCTS::simulator::action_factory af;
  symreg::MCTS::simulator::simulator<symreg::DNN> sim(mab, loss, lp, af, ds, 5, 1, nullptr);

  auto mcts = symreg::MCTS::MCTS(ds, sim, 200); 
  mcts.iterate();
  auto ast = mcts.get_result(); 
  ASSERT_TRUE(ast->is_full());
}

TEST(Reset, ResultsInRootOnlyState) {
  auto mab = std::make_shared<symreg::MCTS::scorer::UCB1>();
  auto ds = symreg::generate_dataset([](int x) { return x; }, 5, 1, 6);
  auto loss = std::make_shared<symreg::loss_fn::NRMSD>();
  auto lp = std::make_shared<symreg::MCTS::simulator::leaf_picker::recursive_random_child_picker>();
  symreg::MCTS::simulator::action_factory af;
  symreg::MCTS::simulator::simulator<symreg::DNN> sim(mab, loss, lp, af, ds, 5, 1, nullptr);

  auto mcts = symreg::MCTS::MCTS(ds, sim, 200); 
  mcts.iterate();
  mcts.reset();
  auto ast = mcts.get_result();
  ASSERT_TRUE(ast->to_string() == "+");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
