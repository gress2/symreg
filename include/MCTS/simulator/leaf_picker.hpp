#pragma once

namespace symreg
{
namespace MCTS
{
namespace simulator
{

/**
 * @brief a CRTP interface for leaf pickers
 */
template <typename T>
class leaf_picker {
  public:
    search_node* pick(search_node*); 
}; 

/**
 * @brief picks a leaf in the MCTS tree using runtime polymorphism
 * @param node the node which use as the base of our leaf search
 * @return a chosen search node (leaf)
 */
template <typename T>
search_node* leaf_picker<T>::pick(search_node* node) {
  return static_cast<T*>(this)->pick_leaf(node);
}

}
}
}

