#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(PickLeaf, ReturnsAValidLeaf) {
  symreg::search_node root(std::make_unique<brick::AST::posit_node>());
  auto three = root.add_child(std::make_unique<brick::AST::number_node>(3));
  auto mul = root.add_child(std::make_unique<brick::AST::multiplication_node>());
  auto x = mul->add_child(std::make_unique<brick::AST::id_node>("x"));
  auto y = mul->add_child(std::make_unique<brick::AST::id_node>("y"));

  symreg::MCTS::simulator::random_leaf_picker lp;

  auto c1 = lp.pick(&root);
  std::cout << c1->get_ast_node()->to_string() << std::endl;
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
