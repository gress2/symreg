#include "MCTS/search_node.hpp"

#include <cmath>
#include <limits>

namespace symreg
{
  /**
   * @brief search node constructor
   *
   * Acquires ownership of an AST node r-value. Initializes visit count (n_),
   * value (v_), parent, and up_link pointers.
   *
   * @param ast_node an r-value reference to an AST node unique pointer which
   * this search node should acquire ownership of.
   */
  search_node::search_node(std::unique_ptr<brick::AST::node>&& ast_node)
    : n_(0), 
      v_(0), 
      depth_(0),
      ast_node_(std::move(ast_node)), 
      parent_(nullptr), 
      up_link_(nullptr)
  {}

  /**
   * @brief search node move constructor
   *
   * moves one search node into another
   *
   * @param other the search node to be moved from
   */
  search_node::search_node(search_node&& other)
    : n_(other.n_),
      v_(other.v_),
      depth_(other.depth_),
      ast_node_(std::move(other.ast_node_)),
      parent_(other.parent_),
      up_link_(other.up_link_),
      children_(std::move(other.children_))
  {}

  /**
   * @brief a setter for the search nodes parent pointer
   * @param parent a pointer to the search node which parent_ should be set to
   */
  void search_node::set_parent(search_node* parent) {
    parent_ = parent;
  }

  /**
   * @brief set the search node's implicit AST parent
   * @param up_link a pointer to the search node which up_link_ should be set to
   */
  void search_node::set_up_link(search_node* up_link) {
    up_link_ = up_link;
  }

  /**
   * @brief creates a graph viz string representation for a search node and
   * its children recursively
   *
   * first, data from the node itself is extracted into a string stream. then,
   * the method loops through the nodes children, writing to the string the value
   * of calling to_gv at each child. The stringstream is converted to a string
   * and returned. 
   *
   * @return a string representation of this search node and all its ancestors
   */
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

  /**
   * @brief given an r-value reference to an AST node unique pointer, 
   * construct and add a child node to a search node
   * @param child_content an r-value reference to a unique pointer for an AST node
   * @return a pointer to the child which was just added
   */
  search_node* search_node::add_child(std::unique_ptr<brick::AST::node>&& child_content) {
    children_.push_back(search_node(std::move(child_content)));
    return &(children_.back());
  }

  /**
   * @brief add a child to a search node given an r-value reference to a search node
   * @param child an r-value reference to a search node
   * @return a pointer to the child which was just added
   */
  search_node* search_node::add_child(search_node&& child) {
    children_.push_back(std::move(child));
    return &(children_.back());
  }

  /**
   * @brief a simple getter for accessing the search node's children
   * @return a reference to the children vector belonging to a search node
   */
  std::vector<search_node>& search_node::get_children() {
    return children_;
  }

  /**
   * @brief tells whether this search node has attached children
   * @return true if the node has no children, false otherwise
   */
  bool search_node::is_leaf_node() const {
    return children_.empty();
  }

  /**
   * @brief returns a pointer to the child of a search node with the highest
   * UCB1 value.
   *
   * UCB1: (v/n) + 2 * sqrt(log(N) / n)
   *
   * Design decision: this metric and its hyperparameters
   * 
   * @return a pointer to the child of a search node with highest UCB1 value
   * if children exist, a nullptr otherwise.
   */
  search_node* search_node::max_UCB1() {
    double max = -std::numeric_limits<double>::infinity();
    search_node* max_node = nullptr;
    for (auto& child : children_) {
      if (child.n_ == 0) {
        return &child;
      }
      double UCB1 = (child.v_/child.n_) + 2 * sqrt(log(n_) / child.n_);
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

  /**
   * @brief a getter for n (the number of times a node has been "visited")
   * @return an integer denoting how many times a search node has been visited
   */
  int search_node::get_n() const {
    return n_;
  }

  /**
   * @brief a setter for n (the number of times a node has been "visited")
   * @param val the value which we wish to set this nodes visit count to
   */
  void search_node::set_n(int val) {
    n_ = val;
  }

  /**
   * @brief a getter for v (a node's value)
   * @return v -- a double
   */
  double search_node::get_v() const {
    return v_;
  }

  /**
   * @brief a setter for v (a node's value)
   * @param val the value which we wish to sit this nodes value to
   */
  void search_node::set_v(double val) {
    v_ = val;
  }

  /**
   * @brief a getter for the AST depth of this search node
   * @return an integer denoting the depth which this node is when converted to AST
   */
  int search_node::get_depth() const {
    return depth_;
  }

  /**
   * @brief a setter for the AST depth of this search node
   * @param val the value with which to use to set depth_
   */
  void search_node::set_depth(int val) {
    depth_ = val;
  }

  /**
   * @brief a getter for a nodes parent connection (in the MCTS sense)
   *
   * Returns a pointer to the search node which this node is a child of. This is not
   * the parent in the AST sense. That parent is given by the "up_link"
   *
   * @return a pointer to the search node which this node is a child of in
   * the tree search tree
   */
  search_node* search_node::parent() {
    return parent_;
  }

  /**
   * @brief tells whether or not the AST node owned by this search node
   * may have children.
   * @return true if the underlying AST node may have children, false if it may note
   */
  bool search_node::is_terminal() const {
    return ast_node_->is_terminal();
  }

  /**
   * @brief a getter for the pointer to the implicit AST parent
   *
   * returns the pointer which this node is a child of in the AST sense. The
   * parent may be well above this node in the MCTS tree.
   * 
   * @return the pointer which this node is a child of in the AST sense 
   */
  search_node* search_node::up_link() {
    return up_link_;
  }

  /**
   * @brief a getter for the search node's contained ast node
   * @return a reference to the unique pointer for the contained ast node
   */
  std::unique_ptr<brick::AST::node>& search_node::ast_node() {
    return ast_node_;
  } 

  /**
   * @brief tells whether or not this node has been rolled out from
   * @return true if the node has been rolled out from, false if not
   */
  bool search_node::visited() const {
    return n_ > 0;
  }
}
