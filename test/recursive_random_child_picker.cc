#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(PickLeaf, ReturnsAValidLeaf) {
  symreg::MCTS::simulator::recursive_random_child_picker rrcp;
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
