#ifndef SYMREG_MCTS_SIMULATOR_RANDOM_LEAF_PICKER_
#define SYMREG_MCTS_SIMULATOR_RANDOM_LEAF_PICKER_

#include "MCTS/simulator/leaf_picker.hpp"

namespace symreg
{
namespace MCTS
{
namespace simulator
{

/**
 * @brief a leaf picker which first builds a vector of all leaves in the
 * tree and then picks at random from the vector
 */
class random_leaf_picker : public leaf_picker<random_leaf_picker> {
  private:
    std::mt19937& mt_;
    void build_leaf_vector(search_node*, std::vector<search_node*>&);
  public:
    random_leaf_picker(std::mt19937&);
    search_node* pick_leaf(search_node*);
};

/**
 * @brief random_leaf_picker constructor
 * @param mt a reference to a marsenne twister instance
 */
random_leaf_picker::random_leaf_picker(std::mt19937& mt)
  : mt_(mt)
{}

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
search_node* random_leaf_picker::pick_leaf(search_node* node) {
  std::vector<search_node*> leaves;
  build_leaf_vector(node, leaves);
  if (leaves.empty()) {
    return nullptr;
  }
  int random = util::get_random_int(0, leaves.size() - 1, mt_);
  return leaves[random];
}


}
}
}

#endif
