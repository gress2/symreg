#ifndef SYMREG_MCTS_TREE_SEARCH_NODE_HPP_
#define SYMREG_MCTS_TREE_SEARCH_NODE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "brick.hpp"

namespace symreg
{
  class search_node {
    private:
      int n_;
      int t_;
      std::unique_ptr<brick::AST::node> ast_node_;
      search_node* parent_;
      search_node* up_link_;
      std::vector<search_node> children_;
    public:
      search_node(std::unique_ptr<brick::AST::node>&&);
      void set_parent(search_node*);
      void set_up_link(search_node*);
      std::string to_gv() const;
      search_node* add_child(std::unique_ptr<brick::AST::node>&&);
      std::vector<search_node>& children();
      bool is_leaf_node() const;
      search_node* max_UCB1();
      double avg_child_val() const;
      int n() const;
      search_node* parent();
      bool is_terminal() const;
      search_node* up_link();
  };
}

#endif
