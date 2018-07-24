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

TEST(Rollout, ResultsInValidAST) {
  symreg::search_node node(std::make_unique<brick::AST::posit_node>());
  symreg::MCTS::simulator::action_factory af(1);
  
  auto ast = symreg::MCTS::simulator::rollout(&node, 4, af);
  ASSERT_TRUE(ast->is_full());
  ASSERT_LE(ast->get_size(), 4);
}

TEST(AddActions, AddsNoActionsIfNoAvailableUpLinks) {
  auto mab = symreg::MCTS::UCB1;
  symreg::dataset ds;
  ds.x = {1, 2, 3};
  ds.y = {4, 5, 6};
  auto loss = symreg::MCTS::bind_loss_fn(symreg::MCTS::NRMSD, ds); 
  auto lp = symreg::MCTS::simulator::recursive_random_child_picker();

  symreg::MCTS::simulator::simulator sim(mab, loss, lp, 4);

  symreg::search_node root(std::make_unique<brick::AST::posit_node>());
  root.add_child(std::make_unique<brick::AST::number_node>(1));
  auto& one = root.get_children().front();
  one.set_up_link(&root);
  one.set_parent(&root);
  ASSERT_FALSE(sim.add_actions(&one));
}

TEST(AddActions, AddsActionsAccordingToDepthLimit1) {
  auto mab = symreg::MCTS::UCB1;
  symreg::dataset ds;
  ds.x = {1, 2, 3};
  ds.y = {4, 5, 6};
  auto loss = symreg::MCTS::bind_loss_fn(symreg::MCTS::NRMSD, ds); 
  auto lp = symreg::MCTS::simulator::recursive_random_child_picker();

  symreg::MCTS::simulator::simulator sim(mab, loss, lp, 4);

  symreg::search_node root(std::make_unique<brick::AST::posit_node>());
  root.add_child(std::make_unique<brick::AST::addition_node>());
  root.set_depth(1);
  auto& addition = root.get_children().front();
  addition.set_up_link(&root);
  addition.set_parent(&root);
  addition.set_depth(2);
  addition.set_unconnected(2);
  ASSERT_TRUE(sim.add_actions(&addition));
  for (auto& child : addition.get_children()) {
    ASSERT_TRUE(child.get_ast_node()->is_terminal());
  }
}

TEST(AddActions, AddsActionsAccordingToDepthLimit2) {
  auto mab = symreg::MCTS::UCB1;
  symreg::dataset ds;
  ds.x = {1, 2, 3};
  ds.y = {4, 5, 6};
  auto loss = symreg::MCTS::bind_loss_fn(symreg::MCTS::NRMSD, ds); 
  auto lp = symreg::MCTS::simulator::recursive_random_child_picker();

  symreg::MCTS::simulator::simulator sim(mab, loss, lp, 9);

  symreg::search_node root(std::make_unique<brick::AST::posit_node>());
  root.add_child(std::make_unique<brick::AST::addition_node>());
  root.set_depth(1);
  auto& addition = root.get_children().front();
  addition.set_up_link(&root);
  addition.set_parent(&root);
  addition.set_depth(2);
  addition.set_unconnected(2);
  ASSERT_TRUE(sim.add_actions(&addition));
  symreg::MCTS::simulator::action_factory af(1);
  ASSERT_EQ(addition.get_children().size(), af.get_set(100).size()); 
}

TEST(Simulate, ExpandsTreeIfPossible) {
  auto mab = symreg::MCTS::UCB1;
  symreg::dataset ds;
  ds.x = {1, 2, 3};
  ds.y = {4, 5, 6};
  auto loss = symreg::MCTS::bind_loss_fn(symreg::MCTS::NRMSD, ds); 
  auto lp = symreg::MCTS::simulator::recursive_random_child_picker();

  symreg::MCTS::simulator::simulator sim(mab, loss, lp, 9);

  symreg::search_node root(std::make_unique<brick::AST::posit_node>());
  root.add_child(std::make_unique<brick::AST::addition_node>());
  root.set_depth(1);
  auto& addition = root.get_children().front();
  addition.set_up_link(&root);
  addition.set_parent(&root);
  addition.set_depth(2);
  addition.set_unconnected(2);

  sim.simulate(&addition, 50);

  ASSERT_TRUE(sim.got_reward_within_thresh() 
      || addition.get_children().size() > 0);
}

TEST(Simulate, DoesntExpandTreeIfDepthMaximized) {
  auto mab = symreg::MCTS::UCB1;
  symreg::dataset ds;
  ds.x = {1, 2, 3};
  ds.y = {4, 5, 6};
  auto loss = symreg::MCTS::bind_loss_fn(symreg::MCTS::NRMSD, ds); 
  auto lp = symreg::MCTS::simulator::recursive_random_child_picker();

  symreg::MCTS::simulator::simulator sim(mab, loss, lp, 2);

  symreg::search_node root(std::make_unique<brick::AST::posit_node>());
  root.add_child(std::make_unique<brick::AST::number_node>(1));
  root.set_depth(1);
  auto& one = root.get_children().front();
  one.set_up_link(&root);
  one.set_parent(&root);
  one.set_depth(2);
  one.set_unconnected(0);

  sim.simulate(&one, 50);
  ASSERT_EQ(one.get_children().size(), 0);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
