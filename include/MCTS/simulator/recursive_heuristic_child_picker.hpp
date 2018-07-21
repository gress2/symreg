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
    search_node* max_heuristic_node(search_node*);
  public:
    recursive_heuristic_child_picker(Heuristic);
    search_node* pick_leaf(search_node*);
}; 

template <class Heuristic>
recursive_heuristic_child_picker<Heuristic>
  ::recursive_heuristic_child_picker(Heuristic h)
  : h_(h)
{}


template <class Heuristic>
search_node* recursive_heuristic_child_picker<Heuristic>::max_heuristic_node(search_node* node) {
  std::vector<search_node*> moves;
  double max = -std::numeric_limits<double>::infinity();
  for (auto& child : node->get_children()) {
    if (child.get_n() == 0) {
      moves.push_back(&child);
      continue;
    }
    double score = h_(child.get_v(), child.get_n(), node->get_n());
    if (score > max) {
      max = score;
      moves.clear();
      moves.push_back(&child);
    } else if (score == max) {
      moves.push_back(&child);
    } 
  }
  auto random = util::get_random_int(0, moves.size() - 1, MCTS::mt); 
  return moves[random];
}

template <class Heuristic>
search_node* recursive_heuristic_child_picker<Heuristic>::pick_leaf(search_node* node) {
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

}
}
}

#endif
