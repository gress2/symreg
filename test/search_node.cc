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

TEST(SetAndGetParent, Case1) {
  search_node parent(std::make_unique<brick::AST::negate_node>());
  search_node child(std::make_unique<brick::AST::number_node>(2));
  child.set_parent(&parent);
  ASSERT_TRUE(child.get_parent() == &parent);
  ASSERT_TRUE(child.get_parent()->get_ast_node()->is_negate());
}

TEST(SetAndGetUpLink, Case1) {
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

TEST(SetScorer, Case1) {
  search_node node(std::make_unique<brick::AST::posit_node>());
  node.set_scorer([](double a, int b, int c) { return a + b + c; });
  ASSERT_TRUE(true);
}

TEST(SetAndGetN, Case1) {
  search_node node(std::make_unique<brick::AST::posit_node>());
  node.set_n(842);
  ASSERT_TRUE(node.get_n() == 842);
}

TEST(SetAndGetV, Case1) {
  search_node node(std::make_unique<brick::AST::posit_node>());
  node.set_v(73.45);
  ASSERT_TRUE(node.get_v() == 73.45);
} 

TEST(SetAndGetDepth, Case1) {
  search_node node(std::make_unique<brick::AST::posit_node>());
  node.set_depth(666);
  ASSERT_TRUE(node.get_depth() == 666);
}

TEST(SetAndGetUnconnected, Case1) {
  search_node node(std::make_unique<brick::AST::posit_node>());
  node.set_unconnected(2000);
  ASSERT_TRUE(node.get_unconnected() == 2000);
}

TEST(SetAndCheckDeadEnd, Case1) {
  search_node node(std::make_unique<brick::AST::posit_node>());
  ASSERT_FALSE(node.is_dead_end());
  node.set_dead_end();
  ASSERT_TRUE(node.is_dead_end());
}

TEST(IsLeafNode, Case1) {
  search_node node(std::make_unique<brick::AST::posit_node>());
  ASSERT_TRUE(node.is_leaf_node());
  node.add_child(std::make_unique<brick::AST::number_node>(2));
  ASSERT_FALSE(node.is_leaf_node());  
} 

TEST(IsTerminal, Case1) {
  search_node node1(std::make_unique<brick::AST::posit_node>());
  ASSERT_FALSE(node1.is_terminal());
  search_node node2(std::make_unique<brick::AST::id_node>("y"));
  ASSERT_TRUE(node2.is_terminal());
}

TEST(IsVisited, Case1) {
  search_node node(std::make_unique<brick::AST::posit_node>());
  ASSERT_FALSE(node.is_visited());
  node.set_n(1);
  ASSERT_TRUE(node.is_visited());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
