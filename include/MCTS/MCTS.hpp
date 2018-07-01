#ifndef SYMREG_MCTS_MCTS_HPP_
#define SYMREG_MCTS_MCTS_HPP_

#include <memory>
#include <random>
#include <unordered_map>

#include "brick.hpp"
#include "MCTS/search_node.hpp"

namespace symreg 
{
  class MCTS {
    private:
      static const int C = 2;
      static const int num_simulations_ = 100;
      search_node root_;
      search_node* curr_;
      double rollout_1(search_node*);
      double rollout_2(search_node*);
      bool add_actions(search_node*);
      void backprop(double, search_node*);
      std::vector<search_node*> get_up_link_targets(search_node*);
      search_node* get_earliest_up_link_target(search_node*);
      std::mt19937 rng_;
      std::unique_ptr<brick::AST::node> get_random_action();
      int get_random(int, int);
      std::shared_ptr<brick::AST::AST> build_ast_upward(search_node*);
      std::unordered_map<std::string, double> symbol_table_;
      std::vector<std::unique_ptr<brick::AST::node>> get_action_set();
      void simulate();
      void make_move();
      search_node* choose_leaf();
    public:
      MCTS();
      void iterate(std::size_t);
      std::string to_gv() const;
      std::unordered_map<std::string, double>& symbol_table();
  };
}

#endif
