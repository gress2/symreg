#include "MCTS/search_node.hpp"

#include <cmath>
#include <limits>

namespace symreg
{
  search_node::search_node(std::unique_ptr<brick::AST::node>&& ast_node)
    : n_(0), 
      t_(0), 
      ast_node_(std::move(ast_node)), 
      parent_(nullptr), 
      up_link_(nullptr)
  {}

  search_node::search_node(search_node&& other)
    : n_(other.n_),
      t_(other.t_),
      ast_node_(std::move(other.ast_node_)),
      parent_(other.parent_),
      up_link_(other.up_link_),
      children_(std::move(other.children_))
  {}

  void search_node::set_parent(search_node* parent) {
    parent_ = parent;
  }

  void search_node::set_up_link(search_node* up_link) {
    up_link_ = up_link;
  }

  std::string search_node::to_gv() const {
    std::stringstream ss;
    auto node_id = ast_node_->get_node_id();
    ss << "  " << node_id << " [label=\"" << ast_node_->get_gv_label() << "\"]" << std::endl;
    if (up_link_) {
      ss << "  " << node_id << " -> " << up_link_->ast_node_->get_node_id() << std::endl;
    }
    for (const search_node& child : children_) {
      ss << "  " << node_id << " -> " << child.ast_node_->get_node_id() << std::endl;
      ss << child.to_gv() << std::endl;
    }

    return ss.str();
  }

  search_node* search_node::add_child(std::unique_ptr<brick::AST::node>&& child_content) {
    children_.push_back(search_node(std::move(child_content)));
    return &(children_.back());
  }

  search_node* search_node::add_child(search_node&& child) {
    children_.push_back(std::move(child));
    return &(children_.back());
  }

    std::vector<search_node>& search_node::get_children() {
      return children_;
    }

    bool search_node::is_leaf_node() const {
      return children_.empty();
    }

    search_node* search_node::max_UCB1() {
      double max = -std::numeric_limits<double>::infinity();
      search_node* max_node = nullptr;
      for (auto& child : children_) {
        if (child.n_ == 0) {
          return &child;
        }
        double UCB1 = (child.t_/child.n_) + 2 * sqrt(log(n_) / child.n_);
        if (UCB1 > max) {
          max = UCB1;
          max_node = &child;
        } 
      }
      if (max_node == nullptr) {
        std::cerr << "Something went wrong in search_node::max_UCB1().";
        std::cerr << " Did the search node have children?" << std::endl;
      }
      return max_node;
    }

    int search_node::get_n() const {
      return n_;
    }

    void search_node::set_n(int val) {
      n_ = val;
    }

    double search_node::get_t() const {
      return t_;
    }

  void search_node::set_t(double val) {
    t_ = val;
  }

  search_node* search_node::parent() {
    return parent_;
  }

  bool search_node::is_terminal() const {
    return ast_node_->is_terminal();
  }

  search_node* search_node::up_link() {
    return up_link_;
  }

  std::unique_ptr<brick::AST::node>& search_node::ast_node() {
    return ast_node_;
  } 

  bool search_node::visited() const {
    return n_ > 0;
  }
}