#include "MCTS/search_node.hpp"

namespace symreg
{
  search_node::search_node(std::unique_ptr<brick::AST::node>&& ast_node)
    : ast_node_(std::move(ast_node))
  {}
}
