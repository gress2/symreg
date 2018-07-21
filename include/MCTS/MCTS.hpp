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
#include "MCTS/lambda_lib.hpp"
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
search_node* choose_move(search_node* node) {
  std::vector<search_node*> moves;
  double max = -std::numeric_limits<double>::infinity();
  for (auto& child : node->get_children()) {
    auto n = child.get_n();
    if (n > max) {
      max = n;
      moves.clear();
      moves.push_back(&child);
    } else if (n == max) {
      moves.push_back(&child);
    }
  }
  auto random = util::get_random_int(0, moves.size() - 1, MCTS::mt);
  return moves[random];
}

template <
  class MAB = decltype(UCB1), 
  class LossFn = decltype(NRMSD),
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
    simulator::simulator<
      decltype(bind_loss_fn(loss_, dataset_)),
      LeafPicker
    > simulator_;
    // HELPERS
    void write_game_state(int) const;
    bool game_over();
  public:
    MCTS(
      int, 
      int, 
      dataset, 
      const MAB& = UCB1, 
      const LossFn& = NRMSD, 
      LeafPicker = simulator::recursive_random_child_picker() 
    );
    void iterate();
    std::string to_gv() const;
    dataset& get_dataset();
    std::shared_ptr<brick::AST::AST> build_result();
    void reset();
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
    simulator_(bind_loss_fn(loss_, dataset_), lp, depth_limit)
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
  for (std::size_t i = 0; i < num_simulations_; i++) {
    if (game_over()) {
      break;
    }
    simulator_.simulate(curr_, num_simulations_);
    search_node* prev = curr_;
    curr_ = choose_move(curr_);
    #if LOG_LEVEL > 0
    log_stream_ << "Iteration: " << i << std::endl;
    write_game_state(i);
    #endif
    if (!curr_) {
      curr_ = prev;
      break;
    }
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
std::shared_ptr<AST> MCTS<MAB, LossFn, LeafPicker>::build_result() {
  return ::symreg::MCTS::simulator::build_ast_upward(curr_);
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
}
  
}
}

#endif
