#ifndef SYMREG_MCTS_SIMULATOR_RECURSIVE_RANDOM_CHILD_PICKER_
#define SYMREG_MCTS_SIMULATOR_RECURSIVE_RANDOM_CHILD_PICKER_

#include "MCTS/simulator/leaf_picker.hpp"

namespace symreg
{
namespace MCTS
{
namespace simulator
{

/**
 * @brief a leaf picker which at every level of the tree chooses a child randomly
 */
class recursive_random_child_picker : public leaf_picker<recursive_random_child_picker> {
  private:
    std::mt19937& mt_;
    search_node* random_child(search_node*);
  public:
    recursive_random_child_picker(std::mt19937&);
    search_node* pick_leaf(search_node*); 
};

/**
 * @brief recursive_random_child_picker constructor
 * @param mt a reference to a marsenne twister instance
 */
recursive_random_child_picker::recursive_random_child_picker(std::mt19937& mt)
  : mt_(mt)
{}

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

  int random = util::get_random_int(0, children.size() - 1, mt_);
  return &children[random]; 
}

/**
 * @brief at each level of the tree, a child is randomly selected until
 * a leaf is found. this is a very different behavior from that of 
 * random_leaf_picker
 * @param node the node from which to start the leaf search
 * @return the randomly selected leaf
 */
search_node* recursive_random_child_picker::pick_leaf(search_node* node) {
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

}
}
}

#endif
