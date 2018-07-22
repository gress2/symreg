#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(BuildASTUpwards, WorksForSingleNodes) {
  symreg::search_node node(std::make_unique<brick::AST::number_node>(3));

  auto ast = symreg::MCTS::simulator::build_ast_upward(&node);

  ASSERT_TRUE(ast->eval() == 3);
}

TEST(BuildASTUpwards, WorksForLargerASTs) {
  symreg::search_node root(std::make_unique<brick::AST::addition_node>());
  root.add_child(std::make_unique<brick::AST::number_node>(1));
  root.add_child(std::make_unique<brick::AST::number_node>(2));

  auto& one = root.get_children().front();
  one.set_up_link(&root);
  one.set_parent(&root);
  auto& two = root.get_children().back();
  two.set_up_link(&root);
  two.set_parent(&one);

  auto ast = symreg::MCTS::simulator::build_ast_upward(&two);
  ASSERT_TRUE(ast->eval() == 3);
}

TEST(GetUpLinkTargets, ReturnsEmptyIfNoTargets) {
  symreg::search_node node(std::make_unique<brick::AST::number_node>(3));
  auto targs = symreg::MCTS::simulator::get_up_link_targets(&node);
  ASSERT_EQ(targs.size(), 0);
}

TEST(GetUpLinkTargets, ReturnsAllLinkableTargets) {
  symreg::search_node node(std::make_unique<brick::AST::addition_node>());
  node.add_child(std::make_unique<brick::AST::multiplication_node>());
  auto& mul = node.get_children().front();
  mul.set_up_link(&node);
  mul.set_parent(&node);
  mul.add_child(std::make_unique<brick::AST::subtraction_node>());
  auto& sub = mul.get_children().front();
  sub.set_up_link(&node);
  sub.set_parent(&mul);
  sub.add_child(std::make_unique<brick::AST::number_node>(3));
  auto& three = sub.get_children().front();
  three.set_up_link(&sub);
  three.set_parent(&sub);

  auto targs = symreg::MCTS::simulator::get_up_link_targets(&three);
  ASSERT_EQ(targs.size(), 2);
  ASSERT_TRUE(targs[0]->get_ast_node()->is_subtraction());
  ASSERT_TRUE(targs[1]->get_ast_node()->is_multiplication());
}

TEST(GetEarliestUpLinkTarget, ReturnsEarliestTargetInMCTSTree) {
  symreg::search_node node(std::make_unique<brick::AST::addition_node>());
  node.add_child(std::make_unique<brick::AST::multiplication_node>());
  auto& mul = node.get_children().front();
  mul.set_up_link(&node);
  mul.set_parent(&node);
  mul.add_child(std::make_unique<brick::AST::subtraction_node>());
  auto& sub = mul.get_children().front();
  sub.set_up_link(&node);
  sub.set_parent(&mul);
  sub.add_child(std::make_unique<brick::AST::number_node>(3));
  auto& three = sub.get_children().front();
  three.set_up_link(&sub);
  three.set_parent(&sub);

  auto targ = symreg::MCTS::simulator::get_earliest_up_link_target(&three);
  ASSERT_TRUE(targ->get_ast_node()->is_multiplication());
}

TEST(GetRandomUpLinkTarget, ReturnsAValidNode) {
  symreg::search_node node(std::make_unique<brick::AST::addition_node>());
  node.add_child(std::make_unique<brick::AST::multiplication_node>());
  auto& mul = node.get_children().front();
  mul.set_up_link(&node);
  mul.set_parent(&node);
  mul.add_child(std::make_unique<brick::AST::subtraction_node>());
  auto& sub = mul.get_children().front();
  sub.set_up_link(&node);
  sub.set_parent(&mul);
  sub.add_child(std::make_unique<brick::AST::number_node>(3));
  auto& three = sub.get_children().front();
  three.set_up_link(&sub);
  three.set_parent(&sub);

  auto targ = symreg::MCTS::simulator::get_random_up_link_target(&three);
  ASSERT_TRUE(targ->get_ast_node()->is_multiplication() ||
      targ->get_ast_node()->is_subtraction());
}

TEST(SetTargetsFromAST, SetsNoTargetsForFullAST) {
  auto ast = std::shared_ptr<brick::AST::AST>(brick::AST::parse("3+7"));
  std::queue<std::shared_ptr<brick::AST::AST>> targets;
  symreg::MCTS::simulator::set_targets_from_ast(ast, targets);
  ASSERT_TRUE(targets.empty());
}

TEST(SetTargetsFromAST, FindsAllTargetsInIncompleteAST) {
  symreg::search_node node(std::make_unique<brick::AST::addition_node>());
  node.add_child(std::make_unique<brick::AST::multiplication_node>());
  auto& mul = node.get_children().front();
  mul.set_up_link(&node);
  mul.set_parent(&node);
  mul.add_child(std::make_unique<brick::AST::subtraction_node>());
  auto& sub = mul.get_children().front();
  sub.set_up_link(&node);
  sub.set_parent(&mul);
  sub.add_child(std::make_unique<brick::AST::number_node>(3));
  auto& three = sub.get_children().front();
  three.set_up_link(&sub);
  three.set_parent(&sub);

  auto ast = symreg::MCTS::simulator::build_ast_upward(&three);
  std::queue<std::shared_ptr<brick::AST::AST>> targets;
  symreg::MCTS::simulator::set_targets_from_ast(ast, targets);
  ASSERT_EQ(targets.size(), 2);
  ASSERT_TRUE(targets.front()->get_node()->is_subtraction());
  ASSERT_TRUE(targets.back()->get_node()->is_multiplication());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
