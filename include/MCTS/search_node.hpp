#ifndef SYMREG_MCTS_TREE_SEARCH_NODE_HPP_
#define SYMREG_MCTS_TREE_SEARCH_NODE_HPP_

#include <memory>
#include <vector>

#include "brick.hpp"

namespace symreg
{
  class search_node {
    private:
      int n_;
      std::unique_ptr<brick::AST::node> ast_node_;
      search_node* parent_;
      search_node* up_link_;
      std::vector<std::vector<search_node*>> children_;
    public:
      search_node(std::unique_ptr<brick::AST::node>&&);
  };
}

#endif