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
  }

  void MCTS::iterate(std::size_t n) {
    for (std::size_t i = 0; i < n; i++) {
      curr_ = &root_;
      while (!curr_->is_leaf_node()) {
        curr_ = curr_->max_UCB1();
      }
      if (curr_->n() == 0) {
        rollout(curr_);
      } else {
        add_actions(curr_);
        curr_ = &(curr_->children()[0]);
        rollout(curr_);
      }
    }
  }

  search_node* MCTS::get_up_link_target(search_node* curr) {
    std::map<search_node*, int> targets; 
    search_node* ancestor = curr;
    while (ancestor != nullptr) {
      if (!ancestor->is_terminal()) {
        if (!targets.count(ancestor)) {
          targets[ancestor] = 0;
        }
      }
      targets[ancestor->up_link()] += 1;
      ancestor = ancestor->parent();
    }
    for (auto r_it = targets.rbegin(); r_it != targets.rend(); ++r_it) {
      if (r_it->second < 2) {
        return r_it->first;
      }
    }
    return nullptr;
  }

  search_node MCTS::get_random_action() {
    int r = get_random(0, 4);
    std::cout << "rand" << std::endl;
    std::cout << r << std::endl;
    if (r == 0) {
      return search_node{std::make_unique<brick::AST::number_node>(3)};
    } else if (r == 1) {
      return search_node{std::make_unique<brick::AST::addition_node>()};
    } else if (r == 2) {
      return search_node{std::make_unique<brick::AST::multiplication_node>()};
    } else if (r == 3) {
      return search_node{std::make_unique<brick::AST::id_node>("z")};
    } else {
      return search_node{std::make_unique<brick::AST::id_node>("y")};
    }
  }

  void MCTS::add_actions(search_node* curr) {
    std::cout << "MCTS::add_actions()" << std::endl;
    curr->add_child(std::make_unique<brick::AST::parens_node>());
    curr->add_child(std::make_unique<brick::AST::negate_node>());
    curr->add_child(std::make_unique<brick::AST::addition_node>());
    curr->add_child(std::make_unique<brick::AST::subtraction_node>());
    curr->add_child(std::make_unique<brick::AST::multiplication_node>());
    curr->add_child(std::make_unique<brick::AST::division_node>());
    curr->add_child(std::make_unique<brick::AST::sin_function_node>());
    curr->add_child(std::make_unique<brick::AST::cos_function_node>());
    curr->add_child(std::make_unique<brick::AST::log_function_node>());
    curr->add_child(std::make_unique<brick::AST::number_node>(1));
    curr->add_child(std::make_unique<brick::AST::number_node>(2));
    curr->add_child(std::make_unique<brick::AST::number_node>(3));
    curr->add_child(std::make_unique<brick::AST::id_node>("x"));
  }

  std::unique_ptr<brick::AST::AST> MCTS::build_ast_upward(search_node* bottom, search_node* base) {
    search_node* cur = bottom;
    std::map<search_node*, std::vector<search_node*>> connections;
    std::map<search_node*, std::unique_ptr<brick::AST::node>> search_to_ast_node;
    while (cur != base) {
      search_node* up_link = cur->up_link(); 
      search_to_ast_node[cur] = std::move(cur->ast_node());
      if (!connections.count(up_link)) {
        connections[up_link] = {cur};
      } else {
        connections[up_link].push_back(cur);
      }
      cur = cur->parent();
    }
  
    // todo: probably "sliced"
    search_to_ast_node[base] = std::make_unique<brick::AST::node>(*(base->ast_node()));
    return std::make_unique<brick::AST::AST>(std::make_unique<brick::AST::number_node>(3));
  }

  void MCTS::rollout(search_node* curr) {
    search_node* rollout_base = curr;
    search_node* up_target = get_up_link_target(curr);

    while (up_target) {
      search_node&& random_action = get_random_action();
      auto child = curr->add_child(std::move(random_action));
      child->set_parent(curr);
      child->set_up_link(up_target);
      up_target = get_up_link_target(child);
      curr = child;
    }

    // no more upward targets, must be a full AST
    std::unique_ptr<brick::AST::AST> ast = build_ast_upward(curr, rollout_base);
    std::cout << ast->eval() << std::endl;
    rollout_base->children().clear();
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
    std::cout << dist(rng_) << std::endl;
    return dist(rng_); 
  }
}
