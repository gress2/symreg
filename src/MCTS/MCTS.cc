#include <iostream>
#include <map>

#include "MCTS/MCTS.hpp"

namespace symreg
{
  MCTS::MCTS()
    : root_(search_node(std::make_unique<brick::AST::posit_node>())),
      curr_(&root_)
  {}

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

  void MCTS::rollout(search_node* curr) {
    search_node* up_target = get_up_link_target(curr);
    if (up_target) {
      auto child = curr->add_child(std::make_unique<brick::AST::number_node>(3));
      child->set_parent(curr);
      child->set_up_link(up_target);
    }
  }

  std::string MCTS::to_gv() const {
    std::stringstream ss;
    ss << "digraph {" << std::endl;
    ss << root_.to_gv();
    ss << "}" << std::endl;
    return ss.str();
  }
}
