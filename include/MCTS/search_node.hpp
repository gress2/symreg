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
      double t_;
      std::unique_ptr<brick::AST::node> ast_node_;
      search_node* parent_;
      search_node* up_link_;
      std::vector<search_node> children_ = {};
    public:
      search_node(std::unique_ptr<brick::AST::node>&&);
      search_node(search_node&&);
      void set_parent(search_node*);
      void set_up_link(search_node*);
      std::string to_gv() const;
      search_node* add_child(std::unique_ptr<brick::AST::node>&&);
      search_node* add_child(search_node&&);
      std::vector<search_node>& children();
      bool is_leaf_node() const;
      search_node* max_UCB1();
      int get_n() const;
      void set_n(int);
      double get_t() const;
      void set_t(double);
      search_node* parent();
      bool is_terminal() const;
      search_node* up_link();
      std::unique_ptr<brick::AST::node>& ast_node();
  };
}

#endif
