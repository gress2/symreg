#ifndef SYMREG_MCTS_MCTS_HPP_
#define SYMREG_MCTS_MCTS_HPP_

#include <memory>

#include "brick.hpp"
#include "MCTS/search_node.hpp"

namespace symreg 
{
  class MCTS {
    private:
      static const int C = 2;
      search_node root_;
      search_node* curr_;
      void rollout(search_node*);
      void add_actions(search_node*);
      search_node* get_up_link_target(search_node*);
    public:
      MCTS();
      void iterate(std::size_t);
      std::string to_gv() const;
  };
}

#endif
