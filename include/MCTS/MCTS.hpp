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
#include "MCTS/search_node.hpp"
#include "MCTS/simulator.hpp"

#define LOG_LEVEL 1


namespace symreg {
  
  auto UCB1 = [](double child_val, int child_n, int parent_n) { 
    return child_val + sqrt(2 * log(parent_n) / child_n); 
  };

  auto MSE = [](dataset& ds, std::shared_ptr<brick::AST::AST>& ast) {
    double sum = 0;
    for (std::size_t i = 0; i < ds.x.size(); i++) {
       auto& xi = ds.x[i];
       sum += std::pow(ast->eval(xi) - ds.y[i], 2); 
    }
    double mse = sum / ds.x.size();
    return mse;
  }; 

  auto NRMSD = [](dataset& ds, std::shared_ptr<brick::AST::AST>& ast) {
    double RMSD = sqrt(MSE(ds, ast));
    double min = *std::min_element(ds.y.begin(), ds.y.end());
    double max = *std::max_element(ds.y.begin(), ds.y.end());
    return RMSD / (max - min);
  };

  namespace MCTS {
    
    using AST = brick::AST::AST;

    template <typename LossFn>
    auto bind_loss_fn(LossFn& loss, dataset& ds) {
      return [&] (std::shared_ptr<brick::AST::AST> ast) {
        return -loss(ds, ast);
      }; 
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
    template <class MAB>
    search_node* choose_move(search_node* node, MAB& mab) {
      double max = -std::numeric_limits<double>::infinity();
      search_node* max_node = nullptr;
      for (auto& child : node->get_children()) {
        if (child.get_n() == 0) {
          return &child;
        }
        double score = mab(child.get_v(), child.get_n(), node->get_n());
        if (score > max) {
          max = score;
          max_node = &child;
        } 
      }
      return max_node;
    }


    template <class MAB = decltype(UCB1), class LossFn = decltype(NRMSD)>
    class MCTS {
      private:
        // MEMBERS
        const int num_simulations_;
        dataset dataset_; 
        search_node root_;
        search_node* curr_;
        MAB mab_;
        LossFn loss_;
        const std::string log_file_;
        std::ofstream log_stream_;
        ::symreg::MCTS::simulator::simulator<decltype(bind_loss_fn(loss_, dataset_))> simulator_;
        // HELPERS
        void write_game_state(int) const;
        bool game_over();
      public:
        MCTS(int, int, dataset, const MAB& = UCB1, const LossFn& = NRMSD);
        void iterate();
        std::string to_gv() const;
        dataset& get_dataset();
        std::shared_ptr<brick::AST::AST> build_result();
    };

    /**
     * @brief MCTS constructor
     * 
     * A root search node is constructed using a unary + operator.
     * This node is automatically expanded with all possible actions.
     * The curr_ pointer is initialized to point at this search node.
     * Additionally, the classes random number generator instance is seeded.
     */
    template <class MAB, class LossFn>
    MCTS<MAB, LossFn>::MCTS(
        int depth_limit,
        int num_simulations,
        dataset ds, 
        const MAB& mab, 
        const LossFn& loss
    )
      : num_simulations_(num_simulations),
        dataset_(ds), 
        root_(search_node(std::make_unique<brick::AST::posit_node>())),
        curr_(&root_),
        mab_(mab),
        loss_(loss),
        log_file_("mcts.log"),
        log_stream_(log_file_),
        simulator_(bind_loss_fn(loss_, dataset_), depth_limit)
    { 
      ::symreg::MCTS::simulator::add_actions(curr_, depth_limit);
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
    template <class MAB, class LossFn>
    void MCTS<MAB, LossFn>::iterate() {
      for (std::size_t i = 0; i < num_simulations_; i++) {
        if (game_over()) {
          break;
        }
        simulator_.simulate(curr_, num_simulations_);
        search_node* prev = curr_;
        curr_ = choose_move(curr_, mab_);
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
    template <class MAB, class LossFn>
    void MCTS<MAB, LossFn>::write_game_state(int iteration) const {
      std::ofstream out_file(std::to_string(iteration) + ".gv");
      out_file << to_gv() << std::endl;
    }

    /**
     * @brief a check to determine whether or not more simulation is possible
     * given the current move
     */
    template <class MAB, class LossFn>
    bool MCTS<MAB, LossFn>::game_over() {
      return curr_->is_dead_end();
    }

    /**
     * @brief A recursive method for generating a graph viz representation for
     * the entire MCTS tree
     *
     * @return the graph viz string representation
     */
    template <class MAB, class LossFn>
    std::string MCTS<MAB, LossFn>::to_gv() const {
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
    template <class MAB, class LossFn>
    dataset& MCTS<MAB, LossFn>::get_dataset() {
      return dataset_;
    }

    /**
     * @brief Returns the AST which this MCTS has currently
     * decided is optimal
     *
     * @return a shared pointer to an AST
     */
    template <class MAB, class LossFn>
    std::shared_ptr<AST> MCTS<MAB, LossFn>::build_result() {
      return ::symreg::MCTS::simulator::build_ast_upward(curr_);
    }
  }
}

#endif
