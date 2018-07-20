#ifndef SYMREG_MCTS_SIMULATOR_SIMULATOR_HPP_
#define SYMREG_MCTS_SIMULATOR_SIMULATOR_HPP_

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <queue> 
#include <random>
#include <unordered_map>
#include <vector>

#include "MCTS/search_node.hpp"
#include "MCTS/simulator/action_factory.hpp"
#include "MCTS/simulator/recursive_random_child_picker.hpp"
#include "MCTS/simulator/random_leaf_picker.hpp"
#include "MCTS/simulator/recursive_heuristic_child_picker.hpp"

namespace symreg
{
namespace MCTS
{
namespace simulator
{
  using AST = brick::AST::AST;

  /**
   * @brief builds an AST starting from a search node to the root of the MCTS
   *
   * Given some search node in the MCTS tree, this method works backward (upward)
   * from the node to the root of the tree, building an actual AST as it goes.
   * 
   * @param bottom the MCTS search node to start building the AST from
   * @return a shared pointer to the root of the AST which was built
   */
  std::shared_ptr<AST> build_ast_upward(search_node* bottom) {
    search_node* cur = bottom;
    search_node* root = nullptr; 
    std::map<search_node*, std::shared_ptr<AST>> search_to_ast;

    while (cur->get_parent()) {
      search_to_ast[cur] = std::make_shared<AST>
        (std::unique_ptr<brick::AST::node>(cur->get_ast_node()->clone()));
      cur = cur->get_parent();
    }

    root = cur;

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
  std::vector<search_node*> get_up_link_targets(search_node* curr) {
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
  search_node* get_earliest_up_link_target(search_node* curr) {
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
  search_node* get_random_up_link_target(search_node* curr, std::mt19937& mt) {
    auto targets = get_up_link_targets(curr);
    if (targets.empty()) {
      return nullptr;
    }
    int random = util::get_random_int(0, targets.size() - 1, mt);
    return targets[random];
  }

  /**
   * @brief finds slots for available children in an AST
   *
   * given an AST starting at ast, recurses the tree to find all
   * AST nodes for which the number of children attached to the node
   * is less than the capacity of the node. found nodes are pushed
   * onto the targets queue.
   *
   * @param ast a shared pointer to an ast to be searched
   * @param targets a reference to a queue of ast shared pointers to
   * be filled
   */
  void set_targets_from_ast(std::shared_ptr<AST> ast, 
      std::queue<std::shared_ptr<AST>>& targets) {
    if (!ast->is_full()) {
      targets.push(ast);
    }
    for (std::shared_ptr<AST>& child : ast->get_children()) {
      set_targets_from_ast(child, targets);
    } 
  }

  /**
   * @brief performs a random rollout starting from a search node in the MCTS
   *
   * first, the implicit AST at curr is built. next, we create a queue of 
   * AST nodes which need to have children added to be "full". while this queue
   * is not empty, we add random AST nodes to this AST, adding to the queue 
   * when we append non-terminal AST nodes. once we manage to build a full/valid
   * AST, we evaluate the AST using the symbol table member of the MCTS class.
   * Rollouts may not exceed the depth_limit_ 
   *
   * Design decision: FIFO method of appending random nodes -- does it matter?
   * Design decision: depth limit
   *
   * @param curr the node to rollout from
   * @return the value of our randomly rolled out AST
   */
  std::shared_ptr<AST> rollout(search_node* curr, int depth_limit, action_factory& af) {
    search_node* rollout_base = curr;
    std::shared_ptr<AST> ast = build_ast_upward(rollout_base);

    std::queue<std::shared_ptr<AST>> targets;
    set_targets_from_ast(ast, targets);

    auto size = ast->get_size();
    auto num_unconnected = ast->get_num_unconnected();

    while (!targets.empty()) {
      // same type of logic as in add_actions(). don't add nodes to the AST which will
      // cause the AST to be larger than max depth (max size)
      auto max_child_arity = depth_limit - (size + num_unconnected);
      std::shared_ptr<AST> targ = targets.front();
      // add actions randomly
      std::shared_ptr<AST> child = targ->add_child(af.get_random(max_child_arity));
      size++;
      num_unconnected += child->vacancy() - 1;
      if (!child->is_terminal()) {
        targets.push(child); 
      }
      if (targ->is_full()) {
        targets.pop();
      }
    }
    return ast;
  }

  /**
   * @brief the method for propagating visit count and node value
   * up the tree to ancestor nodes
   *
   * Starting from a passed node curr, follows the parent links up the
   * tree. At each node, the value of the node is increased by the 
   * passed parameter value. Each node along the way is increased by 1.
   *
   * Design decision: the way this math is done. 
   *
   * @param value the value of the rollout to be propagated upward
   * @param curr a pointer to the leaf node which was rolled out
   */
  void backprop(double value, search_node* curr) {
    while (curr) {
      curr->set_v((curr->get_v() * curr->get_n() + value) / (curr->get_n() + 1)); 
      curr->set_n(curr->get_n() + 1);
      value = curr->get_v();
      curr = curr->get_parent();
    }
  }

  template <class RewardFn, class LeafPicker>
  class simulator {
    private:
      RewardFn reward_;
      int depth_limit_;
      action_factory af_;
      LeafPicker lp_;
      std::mt19937& mt_;
    public:
      simulator(
        const RewardFn&, 
        LeafPicker,
        int,
        std::mt19937&
      );
      void simulate(search_node*, int num_sim); 
      bool add_actions(search_node* curr); 
  };

  template <class RewardFn, class LeafPicker>
  simulator<RewardFn, LeafPicker>::simulator(
    const RewardFn& reward, 
    LeafPicker lp, 
    int depth_limit,
    std::mt19937& mt
  )
    : reward_(reward),
      depth_limit_(depth_limit),
      af_(action_factory{1, mt}),
      lp_(lp),
      mt_(mt)
  {}

  /**
   * @brief Expansion, i.e., given a search node, attaches children nodes for all possible moves
   * from the node. 
   *
   * starting from curr, the tree is searched upward to find nodes which have
   * available slots for children to be attached. for each of the nodes found,
   * all possible actions are attached to curr and up-linked to the the node. curr may only
   * be expanded while there are still possible moves to be made. actions are only added when
   * their addition doesnt lead to ASTs of greater depth than depth_limit_
   *
   * Design decision: depth limit
   *
   * @param curr the node to be expanded
   * @return a boolean denoting whether or not the node was expanded
   */
  template <class RewardFn, class LeafPicker>
  bool simulator<RewardFn, LeafPicker>::add_actions(search_node* curr) {
    // find nodes above in the MCTS tree which need children in the AST sense

    std::vector<search_node*> targets = get_up_link_targets(curr);
    if (targets.empty()) {
      return false;
    }
    auto parent_depth = curr->get_depth(); // how many symbols deep are we in this MCTS path
    auto unconnected = curr->get_unconnected(); // how many children are still needed to complete this AST
    // don't add nodes which will cause us to exceed depth limit
    auto max_child_arity = depth_limit_ - (parent_depth + unconnected);

    // if we're already at max depth, don't add any more nodes
    if (parent_depth >= depth_limit_) {
      return false;
    }
    for (search_node* targ : targets) {
      // we're moving these nodes so have to get a new action set each iteration
      std::vector<std::unique_ptr<brick::AST::node>> 
        actions = af_.get_set(max_child_arity);

      for (auto it = actions.begin(); it != actions.end();) {
        auto child = curr->add_child(std::move(*it));
        child->set_parent(curr);
        child->set_up_link(targ);
        child->set_depth(curr->get_depth() + 1);
        child->set_unconnected(
            curr->get_unconnected() - 1 + child->get_ast_node()->num_children()
        );
        
        // we must erase these from the vector after they are moved otherwise
        // the memory gets freed when the vector goes out of scope. TODO: can we avoid this?
        it = actions.erase(it);
      }
    }
    return true;
  }

  /**
   * @brief The simulation step involving node expansion and rollouts
   * 
   * This function iteratively does the following:
   *  1) A leaf node in the MCTS tree is chosen based on some heuristics
   *  2) If the leaf node has already been rolled out from, the leaf node
   *     is expanded and we set the leaf node pointer to the first child 
   *     added in the expansion
   *  3) A random rollout is performed from the leaf node
   *  4) The value of the rollout is backpropagated up the tree.
   * 
   * Design decision: in step 2, the first child is always chosen
   */
  template <class RewardFn, class LeafPicker>
  void simulator<RewardFn, LeafPicker>::simulate(search_node* curr, int num_sim) {
    for (int i = 0; i < num_sim; i++) {
      search_node* leaf = lp_.pick(curr);
      if (!leaf) {
        continue;
      }

      if (leaf->is_visited()) {
        if (leaf->is_dead_end()) {
          // TODO
        } else if (add_actions(leaf)) {
          leaf = &(leaf->get_children()[0]);
        } else {
          leaf->set_dead_end();
        } 
      }
      double value = reward_(rollout(leaf, depth_limit_, af_));
      backprop(value, leaf);
    }
  }

}
}
}

#endif