#include "MCTS/search_node.hpp"

#include <cmath>
#include <limits>

namespace symreg
{
  search_node::search_node(std::unique_ptr<brick::AST::node>&& ast_node)
    : n_(0), t_(0), ast_node_(std::move(ast_node)), parent_(nullptr), up_link_(nullptr)
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

  void search_node::add_child(std::unique_ptr<brick::AST::node>&& child_content) {
    children_.push_back(search_node(std::move(child_content)));
  }

  std::vector<search_node>& search_node::children() {
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
      double UCB1 = child.avg_child_val() + 2 * sqrt(log(n_) / child.n_);
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

  double search_node::avg_child_val() const {
    double sum = 0;
    for (auto& child : children_) {
      sum += child.t_;
    }
    return sum / children_.size();
  }

  int search_node::n() const {
    return n_;
  }

}
