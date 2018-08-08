#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <queue> 
#include <random>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "MCTS/search_node.hpp"
#include "MCTS/simulator/action_factory.hpp"
#include "MCTS/simulator/leaf_picker.hpp"

namespace symreg
{
namespace MCTS
{
namespace simulator
{
  using AST = brick::AST::AST;

  // HELPERS

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
      return targets.back();
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
  search_node* get_random_up_link_target(search_node* curr) {
    auto targets = get_up_link_targets(curr);
    if (targets.empty()) {
      return nullptr;
    }
    int random = util::get_random_int(0, targets.size() - 1, symreg::mt);
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
   * Design decision: LIFO method of appending random nodes -- does it matter?
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
      curr->set_q((curr->get_q() * curr->get_n() + value) / (curr->get_n() + 1)); 
      curr->set_n(curr->get_n() + 1);
      value = curr->get_q();
      curr = curr->get_parent();
    }
  }

  /**
   * @brief loop up a path in the MCTS tree, increasing visit count by
   * the passed value and each search node
   *
   * @param value the value to increase the nodes' visit counts by
   * @param curr the node to start increasing from
   */
  void increase_visit_upward(int value, search_node* curr) {
    while (curr) {
      curr->set_n(curr->get_n() + value);
      curr = curr->get_parent();
    }
  }

  /**
   * @brief uses secant method to find the number of times the highest
   * scored node must be visited before starting to visit the next
   * highest scoring node.
   *
   * @param function scoring function lambda
   * @param q_term the q value of the highest scored node
   * @param n_term the visit count of the highest scored node
   * @param q_other the q value of the second highest scoring node
   * @param n_other the visit count of the second highest scoring node 
   * @param args a parameter pack for whatever else the scoring function may require
   * @return the visit count which will cause the highest scoring node to become the
   * second highest scoring node
   */
  template <class ScoreFn, class... Args>
  constexpr int compute_tipping_point(
    ScoreFn&& function,
    double q_term,
    int n_term,
    double q_other,
    int n_other,
    int n_total,
    Args&&... args
  )
  {
    // Types and constants
    constexpr double min = std::numeric_limits<double>::min();
    constexpr double max = std::numeric_limits<int>::max();

    // Functions
    const int n = n_total - n_other - n_term;
    auto difference = [=, &function, &args...](auto x) {
      const auto f_terminal = std::forward<ScoreFn>(function)(
        q_term,
        x,
        x + n_other + n,
        std::forward<Args>(args)...
      );
      const auto f_other = std::forward<ScoreFn>(function)(
        q_other,
        n_other,
        x + n_other + n,
        std::forward<Args>(args)...
      );
      return f_terminal - f_other;
    };

    // Initialization
    bool ok = n_term > 0 && n_other > 0 && n_term + n_other <= n_total;
    double x0 = 0;
    double x1 = n_term;
    double x2 = n_other;
    double f0 = 0;
    double f1 = 0;
    int n0 = 0;
    int n1 = std::ceil(x1);
    bool stabilized = false;

    // Secant method
    if (ok && difference(n_term) > 0) {
      f1 = difference(x1);
      do {
        x0 = x1;
        x1 = x2;
        f0 = f1;
        f1 = difference(x1);
        n0 = n1;
        n1 = std::ceil(x1);
        x2 = x1 - f1 * (x1 - x0) / (f1 - f0);
        stabilized = std::signbit(f0 * f1) && n0 == n1;
      } while(!stabilized && std::isnormal(x2 - x1) && x2 < max);
      if (stabilized || std::abs(x2 - x1) < min || std::abs(f1 - f0) < min) {
        n_term = std::max(n_term, n1);
      }
    }

    return n_term;
  }

  /**
   * @brief given a parent node, find the second highest scoring child node
   *
   * @param node the parent node for whose children we are interested in
   * @param _scorer a shared pointer to a search node scorer 
   * @return a pointer to the second highest scoring search node
   */
  search_node* get_second_highest(search_node* node, std::shared_ptr<scorer::scorer>& _scorer) {
    search_node* parent = node->get_parent();
    search_node* second_highest = nullptr;
    double max_score = -1; 
    for (auto& child : parent->get_children()) {
      if (&child == node) {
        continue;
      }
      auto score = _scorer->score(child.get_q(), child.get_n(), parent->get_n());
      if (score > max_score) {
        max_score = score;
        second_highest = &child;
      }
    } 
    return second_highest;
  }

  /**
   * @brief inflates the visit count of a highest scoring terminal node to avoid wasting simulations
   *
   * @param node the highest scoring terminal node which needs its visit count increased
   * @param _scorer a shared pointer to a search_node scorer
   */
  void inflate_visit_count(search_node* node, std::shared_ptr<scorer::scorer>& _scorer) {
    search_node* second_highest = get_second_highest(node, _scorer);
    if (second_highest) {
      auto scorer_lambda = [&] (double child_val, int child_n, int parent_n) {
        return _scorer->score(child_val, child_n, parent_n);
      };

      auto tipping_point = compute_tipping_point(
          scorer_lambda, node->get_q(), node->get_n(), second_highest->get_q(),
          second_highest->get_n(), node->get_parent()->get_n()
      ); 
      int inflate_value = tipping_point - node->get_n();
      increase_visit_upward(inflate_value == 0 ? 1 : inflate_value, node);
    } else {
      std::cout << "Something weird happened." << std::endl;
    }
  } 

  using priq_elem_type = std::pair<std::shared_ptr<AST>, double>;

  static auto priq_cmp = [](const priq_elem_type& lhs, const priq_elem_type& rhs) {
    return lhs.second > rhs.second;  
  };

  static auto priq_elem_sign = [](const priq_elem_type& elem) {
    return elem.second;
  };

  // SIMULATOR

  /**
   * this is the component which actually does simulations within the MCTS
   * algorithm. that is, leaves are chosen within the search tree and expanded
   * or rolled out from
   */ 
  template <class Regressor = symreg::DNN>
  class simulator {
    private:
      std::shared_ptr<scorer::scorer> scorer_;
      std::shared_ptr<loss_fn::loss_fn> loss_fn_;
      std::shared_ptr<leaf_picker::leaf_picker> leaf_picker_;
      action_factory action_factory_;
      dataset& ds_;
      int depth_limit_;
      double early_term_thresh_;
      std::shared_ptr<AST> ast_within_thresh_;
      fixed_priority_queue<priq_elem_type, 
        decltype(priq_cmp), decltype(priq_elem_sign), 20> priq_; 
      Regressor* regr_;
    public:
      // for convenience
      simulator(dataset&);
      // for testing
      simulator(
        std::shared_ptr<scorer::scorer>,
        std::shared_ptr<loss_fn::loss_fn>, 
        std::shared_ptr<leaf_picker::leaf_picker>,
        action_factory,
        dataset&,
        int = 10,
        double = .999,
        Regressor* = nullptr
      );
      // configured with .toml
      simulator(util::config&, dataset&, Regressor*);
      void simulate(search_node*, int num_sim); 
      bool add_actions(search_node* curr); 
      bool got_reward_within_thresh();
      std::shared_ptr<AST> get_ast_within_thresh();
      void reset();
      std::vector<std::shared_ptr<AST>> dump_pri_q();
  };

  /**
   * @brief a very default constructor for simulator
   *
   * @param ds a reference to a datset
   */
  template <class Regressor>
  simulator<Regressor>::simulator(dataset& ds)
    : scorer_(std::make_shared<scorer::UCB1>()),
      loss_fn_(std::make_shared<loss_fn::MAPE>()),
      leaf_picker_(std::make_shared
          <leaf_picker::recursive_heuristic_child_picker<scorer::UCB1>>(scorer::UCB1{})),
      action_factory_(action_factory{}), 
      ds_(ds),
      depth_limit_(8),
      early_term_thresh_(.999),
      ast_within_thresh_(nullptr),
      priq_(priq_cmp, priq_elem_sign),
      regr_(nullptr)
  {}
      
  /**
   * @brief a fully configurable constructor for simulator
   *
   * @param _scorer a shared_ptr to a scorer instance, which
   * determines the score of a search_node in the MCTS tree
   * @param _loss_fn a shared_ptr to a loss function instance;
   * the loss function is used to determine the value of an AST 
   * @param _leaf_picker a shared_ptr to a leaf_picker instance,
   * which determines how leaves in the tree are picked for expansion/rollout
   * @param _action_factor an object which produces valid actions
   * to be taken from each search node state i.e. what AST nodes should
   * be added to the current path
   * @param ds a reference to a datset
   * @param depth_limit this determines how deep a valid AST can be in our search
   * @param early_term_thresh determines how large a reward an AST must have to
   * completely halt the search 
   * @param regr a pointer (or nullptr) to a supervised learner capable of inferring
   * a search nodes value and policy
   */
  template <class Regressor>
  simulator<Regressor>::simulator(
    std::shared_ptr<scorer::scorer> _scorer,
    std::shared_ptr<loss_fn::loss_fn> _loss_fn, 
    std::shared_ptr<leaf_picker::leaf_picker> _leaf_picker, 
    action_factory _action_factory,
    dataset& ds,
    int depth_limit,
    double early_term_thresh,
    Regressor* regr 
  )
    : scorer_(_scorer), 
      loss_fn_(_loss_fn),
      leaf_picker_(_leaf_picker),
      action_factory_(_action_factory),
      ds_(ds),
      depth_limit_(depth_limit),
      early_term_thresh_(early_term_thresh),
      ast_within_thresh_(nullptr),
      priq_(priq_cmp, priq_elem_sign),
      regr_(regr)
  {}

  /**
   * @brief a simulator constructor primarily guided by a util::config
   *
   * pulls properties from a .toml config file to determine how fields of the simulator
   * should be initialized
   *
   * @param cfg a reference to config object (essentially a wrapper around a cpptoml table)
   * @param ds a reference to a datset
   * @param regr a pointer (or nullptr) to a supervised learner capable of inferring
   * a search node's value and policy
   */
  template <class Regressor>
  simulator<Regressor>::simulator(util::config& cfg, dataset& ds, Regressor* regr)
    : scorer_(scorer::get(cfg.get<std::string>("mcts.scorer"))),
      loss_fn_(loss_fn::get(cfg.get<std::string>("mcts.loss_fn"))),
      leaf_picker_(leaf_picker::get(cfg.get<std::string>("mcts.leaf_picker"))),
      action_factory_(action_factory(cfg)),
      ds_(ds),
      depth_limit_(cfg.get<int>("mcts.depth_limit")),
      early_term_thresh_(cfg.get<double>("mcts.early_term_thresh")),
      ast_within_thresh_(nullptr),
      priq_(priq_cmp, priq_elem_sign),
      regr_(regr)
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
   * @param curr the node to be expanded
   * @return a boolean denoting whether or not the node was expanded
   */
  template <class Regressor>
  bool simulator<Regressor>::add_actions(search_node* curr) {
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
        actions = action_factory_.get_set(max_child_arity);
      
      for (auto it = actions.begin(); it != actions.end();) {
        curr->add_child(std::move(*it));
        auto& child = curr->get_children().back();
        child.set_parent(curr);
        child.set_up_link(targ);
        child.set_depth(curr->get_depth() + 1);
        child.set_unconnected(
            curr->get_unconnected() - 1 + child.get_ast_node()->num_children()
        );
        
        // we must erase these from the vector after they are moved otherwise
        // the memory gets freed when the vector goes out of scope
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
  template <class Regressor>
  void simulator<Regressor>::simulate(search_node* curr, int num_sim) {
    for (int i = 0; i < num_sim; i++) {
      search_node* leaf = leaf_picker_->pick(curr);
      if (!leaf) {
        continue;
      }

      if (leaf->is_visited()) {
        if (leaf->is_dead_end()) {
          //inflate_visit_count(leaf, scorer_);
          leaf->set_n(leaf->get_n() + 1);
          continue;
        } else if (add_actions(leaf)) {
          auto& children = leaf->get_children();
          auto random = util::get_random_int(0, children.size() - 1, symreg::mt);
          leaf = &(children[random]);
        } else {
          leaf->set_dead_end();
        } 
      }

      double value;

      if (regr_) {
        value = regr_->inference("state goes here").first; 
        backprop(value, leaf);
      } else {
        auto rollout_ast = rollout(leaf, depth_limit_, action_factory_);
        value = 1 - loss_fn_->loss(ds_, rollout_ast);
        if (std::isnan(value) || std::isinf(value)) {
          value = 0;
        }
        priq_.push(std::make_pair(rollout_ast, value));
        backprop(value, leaf);
        if (value > early_term_thresh_) {
          ast_within_thresh_ = rollout_ast;
          break;
        }
      }
    }
  }

  /**
   * @brief checks whether an AST was encountered whose reward
   * was >= the early stopping threshold
   * @return true if a suitable AST was encountered, false otherwise
   */
  template <class Regressor>
  bool simulator<Regressor>::got_reward_within_thresh() {
    return ast_within_thresh_.get();
  }

  /**
   * @brief a getter for the AST whose reward was within the early stopping threshold
   * @return if an AST whose reward was >= the early stopping threshold
   * was encountered, returns a shared_pointer to this AST. If no such
   * AST has been seen, should return a nullptr
   */
  template <class Regressor>
  std::shared_ptr<AST> simulator<Regressor>::get_ast_within_thresh() {
    return ast_within_thresh_;
  }

  /**
   * @brief resets the state of the simulator, enabling it to be reused
   */
  template <class Regressor>
  void simulator<Regressor>::reset() {
    ast_within_thresh_ = nullptr;
  }

  /**
   * @brief puts all of the AST's in the priority queue in a vector and
   * returns them
   * @return a vector of shared pointers to ASTs. these ASTs will been among
   * the top N highest rewarding ASTs encountered in simulation
   */
  template <class Regressor>
  std::vector<std::shared_ptr<AST>> simulator<Regressor>::dump_pri_q() {
    std::vector<std::shared_ptr<AST>> vec;
    auto pair_ary = priq_.dump();
    for (auto& pair : pair_ary) {
      vec.push_back(pair.first);
    }
    return vec;
  } 

} // simulator
} // MCTS
} // symreg
