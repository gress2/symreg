#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(RandomLeafPicker, PickReturnsAValidLeaf) {
  symreg::search_node root(std::make_unique<brick::AST::posit_node>());
  root.add_child(std::make_unique<brick::AST::number_node>(3));
  root.add_child(std::make_unique<brick::AST::multiplication_node>());
  auto* three = &(root.get_children()[0]);
  auto* mul = &(root.get_children()[1]);
  mul->add_child(std::make_unique<brick::AST::id_node>("x"));
  mul->add_child(std::make_unique<brick::AST::id_node>("y"));
  auto* x = &(mul->get_children()[0]);
  auto* y = &(mul->get_children()[1]);

  symreg::MCTS::simulator::leaf_picker::random_leaf_picker lp;
  auto c1 = lp.pick(&root);
  ASSERT_TRUE(c1 == three || c1 == x || c1 == y); 
  auto c2 = lp.pick(mul);
  ASSERT_TRUE(c2 == x || c2 == y);
  auto c3 = lp.pick(x);
  ASSERT_TRUE(c3 == x);
}

TEST(RHCP, PickReturnsAValidLeaf) {
  auto heuristic = symreg::MCTS::scorer::UCB1();

  symreg::MCTS::simulator::leaf_picker::recursive_heuristic_child_picker rhcp(heuristic);
  symreg::search_node root(std::make_unique<brick::AST::number_node>(0));
  root.add_child(std::make_unique<brick::AST::number_node>(1));
  root.add_child(std::make_unique<brick::AST::number_node>(2));
  
  auto* leaf = rhcp.pick(&root);
  ASSERT_TRUE(leaf->get_ast_node()->is_number());

  leaf = &(root.get_children()[0]); 
  leaf = rhcp.pick(leaf);
  ASSERT_TRUE(leaf->get_ast_node()->is_number());
}

TEST(RHCP, PickPrefersUnvisitedNodes) {
  auto heuristic = symreg::MCTS::scorer::UCB1();
  
  symreg::MCTS::simulator::leaf_picker::recursive_heuristic_child_picker rhcp(heuristic);
  symreg::search_node root(std::make_unique<brick::AST::number_node>(0));
  root.add_child(std::make_unique<brick::AST::number_node>(1));
  root.add_child(std::make_unique<brick::AST::number_node>(2));

  auto& one = root.get_children().front();
  one.set_n(200);
  one.set_q(2000000000);
  auto& two = root.get_children().back();
  two.set_n(0);
  two.set_q(.00001);

  auto* leaf = rhcp.pick(&root);
  ASSERT_TRUE(leaf->get_ast_node()->to_string() == "2"); 
}

TEST(RHCP, PickReturnsLeafMaximizingHeuristic) {
  auto heuristic = symreg::MCTS::scorer::UCB1();
  
  symreg::MCTS::simulator::leaf_picker::recursive_heuristic_child_picker rhcp(heuristic);
  symreg::search_node root(std::make_unique<brick::AST::number_node>(0));
  root.add_child(std::make_unique<brick::AST::number_node>(1));
  root.add_child(std::make_unique<brick::AST::number_node>(2));

  root.set_n(10);
  root.set_q(5);
  auto& one = root.get_children().front();
  one.set_n(8);
  one.set_q(4);
  auto& two = root.get_children().back();
  two.set_n(2);
  two.set_q(9); 

  auto leaf = rhcp.pick(&root);
  std::cout << leaf->get_ast_node()->to_string() << std::endl;
  ASSERT_TRUE(leaf->get_ast_node()->to_string() == "2");
}

TEST(RRCP, PickReturnsAValidLeaf) {
  symreg::MCTS::simulator::leaf_picker::recursive_random_child_picker rrcp;
  symreg::search_node root(std::make_unique<brick::AST::number_node>(0));
  root.add_child(std::make_unique<brick::AST::number_node>(1));
  root.add_child(std::make_unique<brick::AST::number_node>(2));
  
  auto* leaf = rrcp.pick(&root);
  ASSERT_TRUE(leaf->get_ast_node()->is_number());

  leaf = &(root.get_children()[0]); 
  leaf = rrcp.pick(leaf);
  ASSERT_TRUE(leaf->get_ast_node()->is_number());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
