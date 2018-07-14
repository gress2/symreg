#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

using AST = brick::AST::AST;
using search_node = symreg::search_node;

TEST(ConstructionFromAstNode, Case1) {
  search_node node(std::make_unique<brick::AST::number_node>(3));
  ASSERT_TRUE(node.get_ast_node()->is_number()); 
}

TEST(MoveConstruction, Case1) {
  search_node node1(std::make_unique<brick::AST::id_node>("x"));
  node1.set_n(100);
  node1.set_depth(33);
  node1.set_unconnected(66);
  search_node node2(std::move(node1));
  ASSERT_TRUE(node2.get_ast_node()->is_id());
  ASSERT_TRUE(node2.get_n() == 100);
  ASSERT_TRUE(node2.get_depth() == 33);
  ASSERT_TRUE(node2.get_unconnected() == 66);
}

TEST(SetParent, Case1) {
  search_node parent(std::make_unique<brick::AST::negate_node>());
  search_node child(std::make_unique<brick::AST::number_node>(2));
  child.set_parent(&parent);
  ASSERT_TRUE(child.get_parent() == &parent);
  ASSERT_TRUE(child.get_parent()->get_ast_node()->is_negate());
}

TEST(SetUpLink, Case1) {
  search_node up_link(std::make_unique<brick::AST::addition_node>());
  search_node node(std::make_unique<brick::AST::subtraction_node>());
  node.set_up_link(&up_link);
  ASSERT_TRUE(node.get_up_link() == &up_link);
  ASSERT_TRUE(node.get_up_link()->get_ast_node()->is_addition());
}

TEST(AddChildByAstNode, Case1) {
  search_node node(std::make_unique<brick::AST::posit_node>());
  node.add_child(std::make_unique<brick::AST::id_node>("g"));
  ASSERT_FALSE(node.get_children().empty());
  ASSERT_TRUE(node.get_children()[0].get_ast_node()->is_id()); 
}

TEST(AddChildBySearchNode, Case1) {
  search_node node(std::make_unique<brick::AST::posit_node>());
  search_node child(std::make_unique<brick::AST::number_node>(77));
  node.add_child(std::move(child));
  ASSERT_FALSE(node.get_children().empty());
  ASSERT_TRUE(node.get_children()[0].get_ast_node()->is_number()); 
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
