#ifndef SYMREG_MCTS_MCTS_HPP_
#define SYMREG_MCTS_MCTS_HPP_

#include <memory>

#include "brick.hpp"
#include "MCTS/search_node.hpp"

namespace symreg 
{
  class MCTS {
    private:
      search_node root_;
      search_node* curr_;
    public:
      MCTS();
  };
}

#endif