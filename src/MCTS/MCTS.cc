#include <iostream>

#include "MCTS/MCTS.hpp"

namespace symreg
{
  MCTS::MCTS()
    : root_(search_node(std::make_unique<brick::AST::posit_node>())),
      curr_(&root_)
  {}
}
