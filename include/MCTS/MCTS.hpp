#ifndef SYMREG_MCTS_MCTS_HPP_
#define SYMREG_MCTS_MCTS_HPP_

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <queue> 
#include <random>
#include <unordered_map>
#include <vector>

#include "brick.hpp"
#include "dataset.hpp"
#include "MCTS/mt.hpp"
#include "MCTS/search_node.hpp"
#include "MCTS/score_functions.hpp"
#include "MCTS/loss_functions.hpp"
#include "MCTS/util.hpp"
#include "MCTS/simulator/simulator.hpp"

#define LOG_LEVEL 1

namespace symreg 
{
namespace MCTS 
{
  
using AST = brick::AST::AST;

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
search_node* choose_move(search_node* node, double terminal_thresh) {
  std::vector<search_node*> moves;
  std::vector<search_node*> weak_terminals;
  double max = -std::numeric_limits<double>::infinity();
  for (auto& child : node->get_children()) {

    if (child.get_ast_node()->is_terminal()) {
      if (child.get_v() < terminal_thresh) {
        weak_terminals.push_back(&child);
        continue;
      }
    }

    auto n = child.get_n();
    if (n > max) {
      max = n;
      moves.clear();
      moves.push_back(&child);
    } else if (n == max) {
      moves.push_back(&child);
    }
  }
  // this just assures that we always end up with a valid AST
  // from move making
  if (moves.empty()) {
    for (auto term : weak_terminals) {
      auto n = term->get_n();
      if (n > max) {
        max = n;
        moves.clear();
        moves.push_back(term);
      } else if (n == max) {
        moves.push_back(term);
      } 
    }
  }

  auto random = util::get_random_int(0, moves.size() - 1, MCTS::mt);
  return moves[random];
}

template <
  class MAB = decltype(score::UCB1), 
  class LossFn = decltype(loss::NRMSD),
  class LeafPicker = simulator::recursive_random_child_picker
>
class MCTS {
  private:
    // STATIC MEMBERS
    static std::random_device rd_;
    // MEMBERS
    const int num_simulations_;
    dataset dataset_; 
    search_node root_;
    search_node* curr_;
    MAB mab_;
    LossFn loss_;
    const std::string log_file_;
    std::ofstream log_stream_;
    std::shared_ptr<brick::AST::AST> result_ast_;
    double terminal_thresh_ = .999;
    simulator::simulator<
      MAB,
      decltype(bind_loss_fn(loss_, dataset_)),
      LeafPicker
    > simulator_;
    // HELPERS
    void write_game_state(int) const;
    bool game_over();
    std::shared_ptr<brick::AST::AST> build_result();
  public:
    MCTS(
      int, 
      int, 
      dataset, 
      const MAB& = score::UCB1, 
      const LossFn& = loss::NRMSD, 
      LeafPicker = simulator::recursive_random_child_picker() 
    );
    void iterate();
    std::string to_gv() const;
    dataset& get_dataset();
    void reset();
    std::shared_ptr<brick::AST::AST> get_result();
    std::vector<std::shared_ptr<brick::AST::AST>> get_top_n_asts();
};


/**
 * @brief MCTS constructor
 * 
 * A root search node is constructed using a unary + operator.
 * This node is automatically expanded with all possible actions.
 * The curr_ pointer is initialized to point at this search node.
 * Additionally, the classes random number generator instance is seeded.
 */
template <class MAB, class LossFn, class LeafPicker>
MCTS<MAB, LossFn, LeafPicker>::MCTS(
    int depth_limit,
    int num_simulations,
    dataset ds, 
    const MAB& mab, 
    const LossFn& loss,
    LeafPicker lp
)
  : num_simulations_(num_simulations),
    dataset_(ds), 
    root_(search_node(std::make_unique<brick::AST::posit_node>())),
    curr_(&root_),
    mab_(mab),
    loss_(loss),
    log_file_("mcts.log"),
    log_stream_(log_file_),
    result_ast_(nullptr),
    simulator_(mab_, bind_loss_fn(loss_, dataset_), lp, depth_limit)
{ 
  simulator_.add_actions(curr_);
}

/**
 * @brief The driver for all of the MCTS iterations
 * 
 * A simple loop which first runs a simulation step
 * which explores/expands the tree followed by a step
 * which heuristically decides a move to take in the tree.
 * If make_move() returns false, a move was not made in the
 * game, and the game is over.
 * 
 * @param n number determining how many time we wish to loop
 */
template <class MAB, class LossFn, class LeafPicker>
void MCTS<MAB, LossFn, LeafPicker>::iterate() {
  std::size_t i = 0;
  while (true) {
    if (game_over()) {
      break;
    }
    simulator_.simulate(curr_, num_simulations_);

    if (simulator_.got_reward_within_thresh()) {
      result_ast_ = simulator_.get_ast_within_thresh();
      break;
    }

    search_node* prev = curr_;
    curr_ = choose_move(curr_, terminal_thresh_);
    #if LOG_LEVEL > 0
    log_stream_ << "Iteration: " << i << std::endl;
    write_game_state(i);
    #endif
    if (!curr_) {
      curr_ = prev;
      break;
    }
    i++;
  }
}

/**
 * @brief writes the MCTS tree as a gv to file
 * @param iteration an integer which determines the name of the gv file
 */
template <class MAB, class LossFn, class LeafPicker>
void MCTS<MAB, LossFn, LeafPicker>::write_game_state(int iteration) const {
  std::ofstream out_file(std::to_string(iteration) + ".gv");
  out_file << to_gv() << std::endl;
}

/**
 * @brief a check to determine whether or not more simulation is possible
 * given the current move
 */
template <class MAB, class LossFn, class LeafPicker>
bool MCTS<MAB, LossFn, LeafPicker>::game_over() {
  return curr_->is_dead_end();
}

/**
 * @brief A recursive method for generating a graph viz representation for
 * the entire MCTS tree
 *
 * @return the graph viz string representation
 */
template <class MAB, class LossFn, class LeafPicker>
std::string MCTS<MAB, LossFn, LeafPicker>::to_gv() const {
  std::stringstream ss;
  ss << "digraph {" << std::endl;
  ss << root_.to_gv();
  ss << "}" << std::endl;
  return ss.str();
}

/**
 * @brief a simple getter for the MCTS class' symbol table
 *
 * @return a non-const reference to the MCTS class' symbol table
 */
template <class MAB, class LossFn, class LeafPicker>
dataset& MCTS<MAB, LossFn, LeafPicker>::get_dataset() {
  return dataset_;
}

/**
 * @brief Returns the AST which this MCTS has currently
 * decided is optimal
 *
 * @return a shared pointer to an AST
 */
template <class MAB, class LossFn, class LeafPicker>
std::shared_ptr<brick::AST::AST> MCTS<MAB, LossFn, LeafPicker>::build_result() {
  return simulator::build_ast_upward(curr_);
}

template <class MAB, class LossFn, class LeafPicker>
std::shared_ptr<brick::AST::AST> MCTS<MAB, LossFn, LeafPicker>::get_result() {
  if (result_ast_) {
    return result_ast_;
  } else {
    return build_result();
  }
}

/**
 * @brief Resets the state of the MCTS search, allowing the next
 * iterate call to operate from a blank slate
 */
template <class MAB, class LossFn, class LeafPicker>
void MCTS<MAB, LossFn, LeafPicker>::reset() {
  root_.get_children().clear();
  root_.set_v(0);
  root_.set_n(0);
  curr_ = &root_;
  result_ast_ = nullptr;
  simulator_.reset();
}

template <class MAB, class LossFn, class LeafPicker>
std::vector<std::shared_ptr<brick::AST::AST>> 
  MCTS<MAB, LossFn, LeafPicker>::get_top_n_asts() {
  return simulator_.dump_pri_q();
}
  
}
}

#endif
