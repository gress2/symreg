#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(PickLeaf, ReturnsAValidLeaf) {
  symreg::search_node root(std::make_unique<brick::AST::posit_node>());
  root.add_child(std::make_unique<brick::AST::number_node>(3));
  root.add_child(std::make_unique<brick::AST::multiplication_node>());
  auto* three = &(root.get_children()[0]);
  auto* mul = &(root.get_children()[1]);
  mul->add_child(std::make_unique<brick::AST::id_node>("x"));
  mul->add_child(std::make_unique<brick::AST::id_node>("y"));
  auto* x = &(mul->get_children()[0]);
  auto* y = &(mul->get_children()[1]);

  symreg::MCTS::simulator::random_leaf_picker lp;
  auto c1 = lp.pick(&root);
  ASSERT_TRUE(c1 == three || c1 == x || c1 == y); 
  auto c2 = lp.pick(mul);
  ASSERT_TRUE(c2 == x || c2 == y);
  auto c3 = lp.pick(x);
  ASSERT_TRUE(c3 == x);
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
