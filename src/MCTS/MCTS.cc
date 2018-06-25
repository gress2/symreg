#include <iostream>

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

  void MCTS::add_actions(search_node* curr) {
    std::cout << "MCTS::add_actions()" << std::endl;
  }

  void MCTS::rollout(search_node* curr) {
    std::cout << "MCTS::rollout()" << std::endl;

  }

  std::string MCTS::to_gv() const {
    std::stringstream ss;
    ss << "digraph {" << std::endl;
    ss << root_.to_gv();
    ss << "}" << std::endl;
    return ss.str();
  }
}
