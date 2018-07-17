#ifndef SYMREG_MCTS_MCTS_impl_HPP_
#define SYMREG_MCTS_MCTS_impl_HPP_

#include "MCTS/search_node.hpp"

namespace symreg
{
  class MCTS_impl {
    private:
      std::mt19937 rng_;
    public:
      MCTS_impl(search_node*);
      static std::vector<search_node*> get_up_link_targets(search_node*); 
      static search_node* get_earliest_up_link_target(search_node*);
      search_node* get_random_up_link_target(search_node);
      std::unique_ptr<brick::AST::node> get_random_action(int max_arity);
      int get_random(int, int);
      static std::shared_ptr<AST> build_ast_upward(search_node* bottom, search_node* top);
      static std::vector<std::unique_ptr<brick::AST::node>> get_action_set(int max_action_arity);
  };

  MCTS_impl::MCTS_impl()
  {
    rng_.seed(std::random_device()());
  }

  /**
   * @brief finds ancestors of the passed node which don't have enough children
   * in the AST sense. E.g. an addition node should have two children below it.
   *
   * First, this method iterates up the the tree creating a map from search nodes
   * to the descendants pointing to them (children). This map is then iterated over.
   * If a search node in the map has less child connections than its capacity, 
   * the node becomes an available target and pushed onto the returned vector.
   *
   * @param curr A search node for which we wish to find potential parent targets at or above 
   * @return A vector containing potential parent targets for new descendants to link to
   */
  std::vector<search_node*> MCTS_impl::get_up_link_targets(search_node* curr) {
    // create a map of nodes and how many descendant nodes point to it (number of children)
    std::map<search_node*, int> targets; 
    search_node* tmp = curr;
    while (tmp != nullptr) {
      if (!tmp->is_terminal()) { // don't care about terminal nodes
        if (!targets.count(tmp)) {
          targets[tmp] = 0;
        }
      }
      if (tmp->get_up_link()) {
        targets[tmp->get_up_link()] += 1;
      }
      tmp = tmp->get_parent();
    }
    // create a vector of nodes with less children than they should have
    std::vector<search_node*> avail_targets;
    for (auto r_it = targets.rbegin(); r_it != targets.rend(); ++r_it) {
      if (r_it->second < r_it->first->get_ast_node()->num_children()) {
        avail_targets.push_back(r_it->first);
      }
    }
    return avail_targets;
  }

    /**
   * @brief Finds the earliest (higest in the tree) ancestor of a node 
   * which can be used for a parent connection.
   *
   * Simply calls get_up_link_targets() and returns the first search node 
   * 
   * @param curr the node for which we start the parent search
   * @return the earliest parent target in this path of the MCTS tree
   */
  search_node* MCTS_impl::get_earliest_up_link_target(search_node* curr) {
    auto targets = get_up_link_targets(curr);
    if (targets.empty()) {
      return nullptr;
    } else {
      return targets.front();
    }
  }

  /**
   * @brief Gets a random ancestor of a node which may be used for a parent connection
   *
   * Simply calls get_up_link_targets() and chooses one at random
   *
   * @param curr the node from which we start the parent search
   * @return a random parent target in this path of the MCTS tree
   */ 
  search_node* MCTS_impl::get_random_up_link_target(search_node* curr) {
    auto targets = get_up_link_targets(curr);
    if (targets.empty()) {
      return nullptr;
    }
    int random = get_random(0, targets.size() - 1);
    return targets[random];
  }

  /**
   * @brief simply returns a random integer in the range [lower, upper] 
   *
   * build a uniform integer distribution to be used with the MCTS class' Marsenne twister
   *
   * @param lower the lowest possible integer which may be returned
   * @param upper the highest possible integer which may be returned
   * @return a random integer on [lower, upper]
   */
  int MCTS_impl::get_random(int lower, int upper) {
    std::uniform_int_distribution<std::mt19937::result_type> dist(lower, upper);
    return dist(rng_); 
  }

  /**
   * @brief returns a unique pointer to a randomly chosen AST node type
   *
   * A random number is generated and used to return the corresponding node type
   *
   * @param max_arity the maximum arity of the returned node i.e. the maximum number
   * of children the node type may support 
   * @return a unique pointer to a randomly chosen node type
   */
  std::unique_ptr<brick::AST::node> MCTS_impl::get_random_action(int max_arity) {
    std::vector<std::unique_ptr<brick::AST::node>> action_set = get_action_set(max_arity);
    int random = get_random(0, action_set.size() - 1);
    return std::move(action_set[random]);
  }

  /**
   * @brief builds an AST starting from a search node to the root of the MCTS
   *
   * Given some search node in the MCTS tree, this method works backward (upward)
   * from the node to the root of the tree, building an actual AST as it goes.
   * 
   * @param bottom the MCTS search node to start building the AST from
   * @return a shared pointer to the root of the AST which was built
   */
  std::shared_ptr<AST> MCTS_impl::build_ast_upward(search_node* bottom, search_node* top) {
    search_node* cur = bottom;
    search_node* root = top;
    std::map<search_node*, std::shared_ptr<AST>> search_to_ast;

    while (cur != root) {
      search_to_ast[cur] = std::make_shared<AST>
        (std::unique_ptr<brick::AST::node>(cur->get_ast_node()->clone()));
      cur = cur->get_parent();
    }

    search_to_ast[root] = std::make_shared<AST>
      (std::unique_ptr<brick::AST::node>(root->get_ast_node()->clone()));
    cur = bottom;

    while (cur != root) {
      search_to_ast[cur->get_up_link()]->add_child(search_to_ast[cur]);
      cur = cur->get_parent();
    }
    return search_to_ast[root];
  }

  /**
   * @brief returns a vector of unique pointers to all possible node types
   *
   * a vector is created and unique pointers to all AST node types are pushed
   * onto the vector
   *
   * @param max_action_arity the maximum arity of the returned nodes i.e. the maximum number
   * of children the nodes support
   * @return a vector of unique pointers for all possible node types
   */
  std::vector<std::unique_ptr<brick::AST::node>> MCTS_impl::get_action_set(int max_action_arity) {
    std::vector<std::unique_ptr<brick::AST::node>> actions;
    // binary operators
    if (max_action_arity >= 2) {
      actions.push_back(std::make_unique<brick::AST::addition_node>());
      actions.push_back(std::make_unique<brick::AST::multiplication_node>());
    }
    // unary operators
    if (max_action_arity >= 1) {
    }
    // terminals
    for (int a = 1; a < 3; a++) {
      actions.push_back(std::make_unique<brick::AST::number_node>(a));
    }

    for (int i = 0; i < num_dim_; i++) {
      actions.push_back(std::make_unique<brick::AST::id_node>("_x" + std::to_string(i)));
    }

    return actions; 
  }

}

#endif