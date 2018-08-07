#pragma once

#include <memory>

namespace symreg
{
namespace MCTS
{
namespace simulator
{
namespace leaf_picker
{

// LEAF PICKER INTERFACE

class leaf_picker {
  public:
    virtual search_node* pick(search_node*) = 0; 
}; 

// RANDOM LEAF PICKER

/**
 * @brief a leaf picker which first builds a vector of all leaves in the
 * tree and then picks at random from the vector
 */
class random_leaf_picker : public leaf_picker {
  private:
    void build_leaf_vector(search_node*, std::vector<search_node*>&);
  public:
    search_node* pick(search_node*);
};

/**
 * @brief BFS traversal of a tree starting from node. when
 * leaves are encountered, they are pushed onto the leaves
 * vector.
 * @param node current node being traversed
 */
void random_leaf_picker::build_leaf_vector(search_node* node,
    std::vector<search_node*>& leaves) {
  if (node->is_leaf_node()) {
    leaves.push_back(node);
  } else {
    auto& children = node->get_children();
    for (auto& child : children) {
      build_leaf_vector(&child, leaves);
    }
  }
}

/**
 * @brief builds a vector of all leaves in the tree starting
 * from node. then picks one of the nodes completely at random
 * @param node the node to start the leaf search from
 * @return the randomly chosen leaf
 */
search_node* random_leaf_picker::pick(search_node* node) {
  std::vector<search_node*> leaves;
  build_leaf_vector(node, leaves);
  if (leaves.empty()) {
    return nullptr;
  }
  int random = util::get_random_int(0, leaves.size() - 1, symreg::mt);
  return leaves[random];
}

// RECURSIVE HEURISTIC LEAF PICKER

template <class Scorer>
class recursive_heuristic_child_picker : public leaf_picker {
  private:
    Scorer scorer_;
    search_node* max_heuristic_node(search_node*);
  public:
    recursive_heuristic_child_picker(Scorer);
    search_node* pick(search_node*);
}; 

template <class Scorer>
recursive_heuristic_child_picker<Scorer>
  ::recursive_heuristic_child_picker(Scorer scorer)
  : scorer_(scorer)
{}

template <class Scorer>
search_node* recursive_heuristic_child_picker<Scorer>::max_heuristic_node(search_node* node) {
  std::vector<search_node*> moves;
  double max = -std::numeric_limits<double>::infinity();
  for (auto& child : node->get_children()) {
    if (child.get_n() == 0) {
      if (max < std::numeric_limits<double>::infinity()) {
        moves.clear();
      }
      max = std::numeric_limits<double>::infinity();
      moves.push_back(&child);
      continue;
    }
    double score = scorer_.score(child.get_q(), child.get_n(), node->get_n());
    if (score > max) {
      max = score;
      moves.clear();
      moves.push_back(&child);
    } else if (score == max) {
      moves.push_back(&child);
    } 
  }
  auto random = util::get_random_int(0, moves.size() - 1, symreg::mt); 
  return moves[random];
}

template <class Scorer>
search_node* recursive_heuristic_child_picker<Scorer>::pick(search_node* node) {
  while (!node->is_leaf_node()) {
    auto child = max_heuristic_node(node);
    if (child) {
      node = child;
    } else {
      return nullptr;
    }
  }
  return node;
}

// RECURSIVE RANDOM CHILD PICKER

/**
 * @brief a leaf picker which at every level of the tree chooses a child randomly
 */
class recursive_random_child_picker : public leaf_picker {
  private:
    search_node* random_child(search_node*);
  public:
    search_node* pick(search_node*); 
};

/**
 * @brief returns a pointer to a random child of a passed search node
 * @param node the node which we wish to pick a child of
 * @return a pointer to the random child
 */
search_node* recursive_random_child_picker::random_child(search_node* node) {
  auto& children = node->get_children();

  if (children.empty()) {
    return nullptr;
  }

  int random = util::get_random_int(0, children.size() - 1, symreg::mt);
  return &children[random]; 
}

/**
 * @brief at each level of the tree, a child is randomly selected until
 * a leaf is found. this is a very different behavior from that of 
 * random_leaf_picker
 * @param node the node from which to start the leaf search
 * @return the randomly selected leaf
 */
search_node* recursive_random_child_picker::pick(search_node* node) {
  while (!node->is_leaf_node()) {
    auto child = random_child(node);
    if (child) {
      node = child;
    } else {
      return nullptr;
    }
  }

  return node;
}

std::shared_ptr<leaf_picker> get(std::string picker_str) {
  std::string rhcp = "recursive_heuristic_child_picker";
  if (picker_str == "random_leaf") {
    return std::make_shared<random_leaf_picker>();
  } else if (picker_str == "recursive_random_child") {
    return std::make_shared<recursive_random_child_picker>();
  } else if (picker_str.compare(0, rhcp.size(), rhcp) == 0) {
    std::string tmpl = picker_str.substr(rhcp.size() + 1, 
        picker_str.size() - rhcp.size() - 2);  
    if (tmpl == "UCB1") {
      return std::make_shared<recursive_heuristic_child_picker<scorer::UCB1>>(scorer::UCB1{});
    } else {
      return std::make_shared<recursive_heuristic_child_picker<scorer::UCB1>>(scorer::UCB1{});
    }
  } else {
    return std::make_shared<recursive_heuristic_child_picker<scorer::UCB1>>(scorer::UCB1{});
  } 
} 

} // leaf_picker
} // simulator
} // MCTS
} // symreg
