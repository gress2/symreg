#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(PickLeaf, ReturnsAValidLeaf) {
  auto heuristic = [](double a, int b, int c) {
    return a + b + c;
  }; 

  symreg::MCTS::simulator::recursive_heuristic_child_picker rhcp(heuristic);
  symreg::search_node root(std::make_unique<brick::AST::number_node>(0));
  root.add_child(std::make_unique<brick::AST::number_node>(1));
  root.add_child(std::make_unique<brick::AST::number_node>(2));
  
  auto* leaf = rhcp.pick(&root);
  ASSERT_TRUE(leaf->get_ast_node()->is_number());

  leaf = &(root.get_children()[0]); 
  leaf = rhcp.pick(leaf);
  ASSERT_TRUE(leaf->get_ast_node()->is_number());
}

TEST(PickLeaf, PrefersUnvisitedNodes) {
  auto heuristic = [](double a, int b, int c) {
    return a + b + c;
  }; 
  
  symreg::MCTS::simulator::recursive_heuristic_child_picker rhcp(heuristic);
  symreg::search_node root(std::make_unique<brick::AST::number_node>(0));
  root.add_child(std::make_unique<brick::AST::number_node>(1));
  root.add_child(std::make_unique<brick::AST::number_node>(2));

  auto& one = root.get_children().front();
  one.set_n(200);
  one.set_v(2000000000);
  auto& two = root.get_children().back();
  two.set_n(0);
  two.set_v(.00001);

  auto* leaf = rhcp.pick(&root);
  ASSERT_TRUE(leaf->get_ast_node()->to_string() == "2"); 
}

TEST(PickLeaf, ReturnsLeafMaximizingHeuristic) {
  auto heuristic = [](double a, int b, int c) {
    return a + b + c;
  }; 
  
  symreg::MCTS::simulator::recursive_heuristic_child_picker rhcp(heuristic);
  symreg::search_node root(std::make_unique<brick::AST::number_node>(0));
  root.add_child(std::make_unique<brick::AST::number_node>(1));
  root.add_child(std::make_unique<brick::AST::number_node>(2));

  root.set_n(10);
  root.set_v(5);
  auto& one = root.get_children().front();
  one.set_n(8);
  one.set_v(4);
  auto& two = root.get_children().back();
  two.set_n(2);
  two.set_v(9); 

  auto leaf = rhcp.pick(&root);
  ASSERT_TRUE(leaf->get_ast_node()->to_string() == "1");
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
