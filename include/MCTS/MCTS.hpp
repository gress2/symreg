#pragma once

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
#include "training_example.hpp"
#include "MCTS/search_node.hpp"
#include "MCTS/scorer.hpp"
#include "MCTS/loss.hpp"
#include "util.hpp"
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
      if (child.get_q() < terminal_thresh) {
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

  auto random = util::get_random_int(0, moves.size() - 1, symreg::mt);
  return moves[random];
}

/**
 * @brief the actual coordinator for monte carlo tree search
 */
template <class Regressor = symreg::DNN>
class MCTS {
  private:
    // MEMBERS
    const int num_simulations_;
    dataset& dataset_; 
    search_node root_;
    search_node* curr_;
    std::ofstream log_stream_;
    std::shared_ptr<brick::AST::AST> result_ast_;
    double terminal_thresh_ = .999;
    simulator::simulator<Regressor> simulator_;
    training_examples examples_;
    // HELPERS
    void write_game_state(int) const;
    bool game_over();
    std::shared_ptr<brick::AST::AST> build_current_ast();
  public:
    // composable constructor for testability
    MCTS(dataset&, simulator::simulator<Regressor>, int);
    // .toml configurable
    MCTS(dataset&, Regressor*, util::config);
    void iterate();
    std::string to_gv() const;
    dataset& get_dataset();
    void reset();
    std::shared_ptr<brick::AST::AST> get_result();
    std::vector<std::shared_ptr<brick::AST::AST>> get_top_n_asts();
    training_examples get_training_examples() const;
};

/**
 * @brief an MCTS constructor which allows you to inject some
 * arbitrary simulator. likely this is only useful for writing
 * tests.
 * @param ds a reference to a dataset
 * @param simulator a simulator instance
 * @param num_simulations the number of times you want the simulator
 * to simulate between moves
 */
template <class Regressor>
MCTS<Regressor>::MCTS(
    dataset& ds,
    simulator::simulator<Regressor> _simulator,
    int num_simulations
)
  : num_simulations_(num_simulations),
    dataset_(ds), 
    root_(search_node(std::make_unique<brick::AST::posit_node>())),
    curr_(&root_),
    log_stream_("mcts.log"),
    result_ast_(nullptr),
    simulator_(_simulator)
{ 
  simulator_.add_actions(curr_);
}

/**
 * @brief .toml configurable MCTS constructor
 * @param ds a reference to a dataset
 * @param regr a pointer to a regressor capable of evaluating a search nodes
 * value and policy
 * @param cfg a wrapper around a cpptoml table
 */ 
template <class Regressor>
MCTS<Regressor>::MCTS(dataset& ds, Regressor* regr, util::config cfg)
  : num_simulations_(cfg.get<int>("mcts.num_simulations")),
    dataset_(ds),
    root_(search_node(std::make_unique<brick::AST::posit_node>())),
    curr_(&root_),
    log_stream_(cfg.get<std::string>("logging.file")),
    result_ast_(nullptr),
    simulator_(simulator::simulator<Regressor>(cfg, ds, regr)) 
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
template <class Regressor>
void MCTS<Regressor>::iterate() {
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
    examples_.push_back(training_example{build_current_ast()->to_string(), curr_->get_pi(), 0});
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
  // assign rewards to examples
  auto final_ast = get_result();
  auto final_reward = 3; 
  for (auto& ex : examples_) {
    ex.reward = final_reward;
  }

}

/**
 * @brief writes the MCTS tree as a gv to file
 * @param iteration an integer which determines the name of the gv file
 */
template <class Regressor>
void MCTS<Regressor>::write_game_state(int iteration) const {
  std::ofstream out_file(std::to_string(iteration) + ".gv");
  out_file << to_gv() << std::endl;
}

/**
 * @brief a check to determine whether or not more simulation is possible
 * given the current move
 */
template <class Regressor>
bool MCTS<Regressor>::game_over() {
  return curr_->is_dead_end();
}

/**
 * @brief A recursive method for generating a graph viz representation for
 * the entire MCTS tree
 *
 * @return the graph viz string representation
 */
template <class Regressor>
std::string MCTS<Regressor>::to_gv() const {
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
template <class Regressor>
dataset& MCTS<Regressor>::get_dataset() {
  return dataset_;
}

/**
 * @brief Returns the AST which this MCTS has currently
 * decided is optimal
 *
 * @return a shared pointer to an AST
 */
template <class Regressor>
std::shared_ptr<brick::AST::AST> MCTS<Regressor>::build_current_ast() {
  return simulator::build_ast_upward(curr_);
}

/**
 * @brief returns the best AST the search was able to find.
 * @return if the simulator encountered an AST whose value
 * was within the early termination threshold, that AST is returned.
 * otherwise, we simply return the AST that was formed from traditional
 * move making within the tree.
 */
template <class Regressor>
std::shared_ptr<brick::AST::AST> MCTS<Regressor>::get_result() {
  if (result_ast_) {
    return result_ast_;
  } else {
    return build_current_ast();
  }
}

/**
 * @brief Resets the state of the MCTS search, allowing the next
 * iterate call to operate from a blank slate
 */
template <class Regressor>
void MCTS<Regressor>::reset() {
  root_.get_children().clear();
  root_.set_q(0);
  root_.set_n(0);
  curr_ = &root_;
  result_ast_ = nullptr;
  simulator_.reset();
}

/**
 * @brief gets a vector dump of the AST priority queue
 * being maintained by the simulator.
 * @return a vector of the top N best ASTs encountered
 * by the simulator
 */
template <class Regressor>
std::vector<std::shared_ptr<brick::AST::AST>> 
  MCTS<Regressor>::get_top_n_asts() {
  return simulator_.dump_pri_q();
}

/**
 * @brief a getter for the training_examples needed to
 * train the regressor/neural network about MCTS states.
 * @return a copy of the training examples vector
 */
template <class Regressor>
training_examples MCTS<Regressor>::get_training_examples() const {
  return examples_;
}
  
}
}
