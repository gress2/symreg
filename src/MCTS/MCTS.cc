#include <iostream>
#include <map>

#include "MCTS/MCTS.hpp"

namespace symreg
{
  MCTS::MCTS()
    : root_(search_node(std::make_unique<brick::AST::posit_node>())),
      curr_(&root_)
  {
    rng_.seed(std::random_device()()); 
    add_actions(curr_);
  }

  void MCTS::simulate() {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    for (std::size_t i = 0; i < num_simulations_; i++) {
      search_node* leaf = choose_leaf();
      if (leaf->visited()) {
        if (add_actions(leaf)) {
          leaf = &(leaf->get_children()[0]);
        } else {
          std::cout << "no actions added to this leaf" << std::endl;
        }
      }
      double value = rollout(leaf);
      backprop(value, leaf);
    }
  }

  void MCTS::iterate(std::size_t n) {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    for (std::size_t i = 0; i < n; i++) {
      simulate();
      make_move();
    }
  }

  void MCTS::make_move() {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    curr_ = curr_->max_UCB1();
  }

  search_node* MCTS::choose_leaf() {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    search_node* leaf = curr_;
    while (!leaf->is_leaf_node()) {
      leaf = leaf->max_UCB1();
    }
    return leaf;
  }

  void MCTS::backprop(double value, search_node* curr) {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    while (curr) {
      curr->set_t(curr->get_t() + value);
      curr->set_n(curr->get_n() + 1);
      curr = curr->parent();
    }
  }

  std::vector<search_node*> MCTS::get_up_link_targets(search_node* curr) {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    std::map<search_node*, int> targets; 
    search_node* ancestor = curr;
    while (ancestor != nullptr) {
      if (!ancestor->is_terminal()) {
        if (!targets.count(ancestor)) {
          targets[ancestor] = 0;
        }
      }
      if (ancestor->up_link()) {
        targets[ancestor->up_link()] += 1;
      }
      ancestor = ancestor->parent();
    }
    std::vector<search_node*> avail_targets;
    for (auto r_it = targets.rbegin(); r_it != targets.rend(); ++r_it) {
      if (r_it->second < r_it->first->ast_node()->num_children()) {
        avail_targets.push_back(r_it->first);
      }
    }
    return avail_targets;
  }

  search_node* MCTS::get_earliest_up_link_target(search_node* curr) {
    auto targets = get_up_link_targets(curr);
    if (targets.empty()) {
      return nullptr;
    } else {
      return targets.front();
    }
  }

  search_node MCTS::get_random_action() {
    int r = get_random(0, 4);
    if (r == 0) {
      return search_node{std::make_unique<brick::AST::number_node>(3)};
    } else if (r == 1) {
      return search_node{std::make_unique<brick::AST::addition_node>()};
    } else if (r == 2) {
      return search_node{std::make_unique<brick::AST::multiplication_node>()};
    } else if (r == 3) { return search_node{std::make_unique<brick::AST::id_node>("z")};
    } else {
      return search_node{std::make_unique<brick::AST::id_node>("y")};
    }
  }

  std::vector<std::unique_ptr<brick::AST::node>> MCTS::get_action_set() {
    std::vector<std::unique_ptr<brick::AST::node>> actions;
    actions.push_back(std::make_unique<brick::AST::addition_node>());
    actions.push_back(std::make_unique<brick::AST::number_node>(3));
    actions.push_back(std::make_unique<brick::AST::multiplication_node>());
    return actions; 
  }

  bool MCTS::add_actions(search_node* curr) {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    std::vector<search_node*> targets = get_up_link_targets(curr);
    if (targets.empty()) {
      return false;
    }
    std::vector<std::unique_ptr<brick::AST::node>> actions = get_action_set();
    for (search_node* targ : targets) {
      for (auto it = actions.begin(); it != actions.end();) {
        auto child = curr->add_child(std::move(*it));
        child->set_parent(curr);
        child->set_up_link(targ);
        it = actions.erase(it);
      }
    }
    return true;
  }

  std::shared_ptr<brick::AST::AST> MCTS::build_ast_upward(search_node* bottom) {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    search_node* cur = bottom;
    search_node* root = &root_;
    std::map<search_node*, std::shared_ptr<brick::AST::AST>> search_to_ast;

    while (cur != root) {
      search_to_ast[cur] = std::make_shared<brick::AST::AST>
        (std::unique_ptr<brick::AST::node>(cur->ast_node()->clone()));
      cur = cur->parent();
    }

    search_to_ast[root] = std::make_shared<brick::AST::AST>
      (std::unique_ptr<brick::AST::node>(root->ast_node()->clone()));
    cur = bottom;

    while (cur != root) {
      search_to_ast[cur->up_link()]->add_child(search_to_ast[cur]);
      cur = cur->parent();
    }
    return search_to_ast[root];
  }

  double MCTS::rollout(search_node* curr) {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    search_node* rollout_base = curr;
    search_node* up_target = get_earliest_up_link_target(curr);

    while (up_target) {
      search_node&& random_action = get_random_action();
      auto child = curr->add_child(std::move(random_action));
      child->set_parent(curr);
      child->set_up_link(up_target);
      up_target = get_earliest_up_link_target(child);
      curr = child;
    }

    // no more upward targets, must be a full AST
    std::shared_ptr<brick::AST::AST> ast = build_ast_upward(curr);
    double value = ast->eval(&symbol_table_);
    rollout_base->get_children().clear();
    return value;
  }

  std::string MCTS::to_gv() const {
    std::stringstream ss;
    ss << "digraph {" << std::endl;
    ss << root_.to_gv();
    ss << "}" << std::endl;
    return ss.str();
  }

  int MCTS::get_random(int lower, int upper) {
    std::uniform_int_distribution<std::mt19937::result_type> dist(lower, upper);
    return dist(rng_); 
  }

  std::unordered_map<std::string, double>& MCTS::symbol_table() {
    return symbol_table_;
  }
}
