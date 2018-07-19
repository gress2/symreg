#ifndef SYMREG_MCTS_SIMULATOR_RECURSIVE_HEURISTIC_CHILD_PICKER_
#define SYMREG_MCTS_SIMULATOR_RECURSIVE_HEURISTIC_CHILD_PICKER_

#include "MCTS/simulator/leaf_picker.hpp"

namespace symreg
{
namespace MCTS
{
namespace simulator
{

template <class Heuristic>
class recursive_heuristic_child_picker 
  : public leaf_picker<recursive_heuristic_child_picker<Heuristic>> {
  private:
    Heuristic h_;
  public:
    recursive_heuristic_child_picker(Heuristic);
    search_node* pick_leaf(search_node*);
}; 

template <class Heuristic>
recursive_heuristic_child_picker<Heuristic>::recursive_heuristic_child_picker(Heuristic h)
  : h_(h)
{}

template <class Heuristic>
search_node* recursive_heuristic_child_picker<Heuristic>::pick_leaf(search_node* node) {
  while (!node->is_leaf_node()) {
    auto child = choose_move(node, h_);
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
