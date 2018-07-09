#ifndef SYMREG_MCTS_MCTS_HPP_
#define SYMREG_MCTS_MCTS_HPP_

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

using AST = brick::AST::AST;

namespace symreg 
{
  auto UCB1 = [](double child_val, int child_n, int parent_n) { 
    return child_val / child_n + sqrt(2 * log(parent_n) / child_n); 
  };

  auto MSE = [](dataset& ds, std::shared_ptr<brick::AST::AST>& ast) {
    double sum = 0;
    for (std::size_t i = 0; i < ds.x.size(); i++) {
       auto& xi = ds.x[i];
       sum += std::pow(ast->eval(xi) - ds.y[i], 2); 
    }
    double mse = sum / ds.x.size();
    std::cout << ast->to_string() << std::endl;
    std::cout << "Loss: " << mse << std::endl;
    return -mse;
  }; 

  template <class MAB = decltype(UCB1), class ScoreFn = decltype(MSE)>
  class MCTS {
    private:
      // STATIC MEMBERS
      static const int depth_limit_ = 7;
      static const int num_simulations_ = 20;
      // MEMBERS
      search_node root_;
      search_node* curr_;
      MAB mab_fn_;
      ScoreFn score_fn_;
      std::mt19937 rng_;
      dataset dataset_; 
      int num_dim_;
      // PRIMARY FUNCTIONALITIES
      double rollout(search_node*);
      bool add_actions(search_node*);
      void backprop(double, search_node*);
      void simulate();
      bool make_move();
      search_node* choose_leaf();
      // HELPERS
      std::vector<search_node*> get_up_link_targets(search_node*);
      search_node* get_earliest_up_link_target(search_node*);
      search_node* get_random_up_link_target(search_node*);
      std::unique_ptr<brick::AST::node> get_random_action(int);
      int get_random(int, int);
      std::shared_ptr<brick::AST::AST> build_ast_upward(search_node*);
      std::vector<std::unique_ptr<brick::AST::node>> get_action_set(int);
      search_node* max_score_child(search_node*);
      // TEMPLATE CONFIGURABLE
      double multi_armed_bandit(double, int, int);
      double score(std::shared_ptr<brick::AST::AST>&);
    public:
      MCTS(int, const MAB& = UCB1, const ScoreFn& = MSE);
      void iterate(std::size_t);
      std::string to_gv() const;
      dataset& dataset();
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
  template <class MAB, class ScoreFn>
  MCTS<MAB, ScoreFn>::MCTS(int num_dim, const MAB& mab, const ScoreFn& score_fn)
    : root_(search_node(std::make_unique<brick::AST::posit_node>())),
      curr_(&root_),
      mab_fn_(mab),
      score_fn_(score_fn),
      num_dim_(num_dim)
  { 
    rng_.seed(std::random_device()()); 
    add_actions(curr_);
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
  template <class MAB, class ScoreFn>
  void MCTS<MAB, ScoreFn>::simulate() {
    for (std::size_t i = 0; i < num_simulations_; i++) {
      search_node* leaf = choose_leaf();
      if (leaf->visited()) {
        if (add_actions(leaf)) {
          leaf = &(leaf->get_children()[0]);
        }      
      }
      double value = rollout(leaf);
      backprop(value, leaf);
    }
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
  template <class MAB, class ScoreFn>
  void MCTS<MAB, ScoreFn>::iterate(std::size_t n) {
    for (std::size_t i = 0; i < n; i++) {
      std::cout << "Iteration: " << i << std::endl;
      simulate();
      bool did_move = make_move();
      if (!did_move) {
        break;
      }
    }
  }

  /**
   * @brief The method for actually making moves within 
   * the MCTS algorithm.
   *
   * This simply selects the next move by choosing the child
   * node of the current node with the highest UCB1 value.
   *
   * Design decision: this method
   *
   * @return a boolean telling whether or not a move was made
   */
  template <class MAB, class ScoreFn>
  bool MCTS<MAB, ScoreFn>::make_move() {
    search_node* next = max_score_child(curr_);
    if (next) {
      curr_ = next;
      return true;
    } 
    return false;
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
  template <class MAB, class ScoreFn>
  search_node* MCTS<MAB, ScoreFn>::max_score_child(search_node* node) {
    double max = -std::numeric_limits<double>::infinity();
    search_node* max_node = nullptr;
    for (auto& child : node->get_children()) {
      if (child.get_n() == 0) {
        return &child;
      }
      double score = multi_armed_bandit(child.get_v(), child.get_n(), node->get_n());
      if (score > max) {
        max = score;
        max_node = &child;
      } 
    }
    return max_node;
  }

  /**
   * @brief Chooses a leaf node to rollout/expand in the
   * simulation step.
   *
   * Starting from the current node of the algorithm, iterates
   * downward through tree by always choosing the child with
   * the highest UCB1 value
   *
   * Design decision: this method
   */
  template <class MAB, class ScoreFn>
  search_node* MCTS<MAB, ScoreFn>::choose_leaf() {
    search_node* leaf = curr_;
    while (!leaf->is_leaf_node()) {
      leaf = max_score_child(leaf);
    }
    return leaf;
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
  template <class MAB, class ScoreFn>
  void MCTS<MAB, ScoreFn>::backprop(double value, search_node* curr) {
    while (curr) {
      curr->set_v(curr->get_v() + value);
      curr->set_n(curr->get_n() + 1);
      curr = curr->parent();
    }
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
  template <class MAB, class ScoreFn>
  std::vector<search_node*> MCTS<MAB, ScoreFn>::get_up_link_targets(search_node* curr) {
    // create a map of nodes and how many descendant nodes point to it (number of children)
    std::map<search_node*, int> targets; 
    search_node* tmp = curr;
    while (tmp != nullptr) {
      if (!tmp->is_terminal()) { // don't care about terminal nodes
        if (!targets.count(tmp)) {
          targets[tmp] = 0;
        }
      }
      if (tmp->up_link()) {
        targets[tmp->up_link()] += 1;
      }
      tmp = tmp->parent();
    }
    // create a vector of nodes with less children than they should have
    std::vector<search_node*> avail_targets;
    for (auto r_it = targets.rbegin(); r_it != targets.rend(); ++r_it) {
      if (r_it->second < r_it->first->ast_node()->num_children()) {
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
  template <class MAB, class ScoreFn>
  search_node* MCTS<MAB, ScoreFn>::get_earliest_up_link_target(search_node* curr) {
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
  template <class MAB, class ScoreFn>
  search_node* MCTS<MAB, ScoreFn>::get_random_up_link_target(search_node* curr) {
    auto targets = get_up_link_targets(curr);
    if (targets.empty()) {
      return nullptr;
    }
    int random = get_random(0, targets.size() - 1);
    return targets[random];
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
  template <class MAB, class ScoreFn>
  std::unique_ptr<brick::AST::node> MCTS<MAB, ScoreFn>::get_random_action(int max_arity) {
    std::vector<std::unique_ptr<brick::AST::node>> action_set = get_action_set(max_arity);
    int random = get_random(0, action_set.size() - 1);
    return std::move(action_set[random]);
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
  template <class MAB, class ScoreFn>
  std::vector<std::unique_ptr<brick::AST::node>> MCTS<MAB, ScoreFn>::get_action_set(int max_action_arity) {
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
    actions.push_back(std::make_unique<brick::AST::number_node>(3));

    for (int i = 0; i < num_dim_; i++) {
      actions.push_back(std::make_unique<brick::AST::id_node>("_x" + std::to_string(i)));
    }

    return actions; 
  }

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
  template <class MAB, class ScoreFn>
  bool MCTS<MAB, ScoreFn>::add_actions(search_node* curr) {
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
        actions = get_action_set(max_child_arity);

      for (auto it = actions.begin(); it != actions.end();) {
        auto child = curr->add_child(std::move(*it));
        child->set_parent(curr);
        child->set_up_link(targ);
        child->set_depth(curr->get_depth() + 1);
        child->set_unconnected(
            curr->get_unconnected() - 1 + child->ast_node()->num_children()
        );
        // we must erase these from the vector after they are moved otherwise
        // the memory gets freed when the vector goes out of scope. TODO: can we avoid this?
        it = actions.erase(it);
      }
    }
    return true;
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
  template <class MAB, class ScoreFn>
  std::shared_ptr<AST> MCTS<MAB, ScoreFn>::build_ast_upward(search_node* bottom) {
    search_node* cur = bottom;
    search_node* root = &root_;
    std::map<search_node*, std::shared_ptr<AST>> search_to_ast;

    while (cur != root) {
      search_to_ast[cur] = std::make_shared<AST>
        (std::unique_ptr<brick::AST::node>(cur->ast_node()->clone()));
      cur = cur->parent();
    }

    search_to_ast[root] = std::make_shared<AST>
      (std::unique_ptr<brick::AST::node>(root->ast_node()->clone()));
    cur = bottom;

    while (cur != root) {
      search_to_ast[cur->up_link()]->add_child(search_to_ast[cur]);
      cur = cur->parent();
    }
    return search_to_ast[root];
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
  template <class MAB, class ScoreFn>
  double MCTS<MAB, ScoreFn>::rollout(search_node* curr) {
    search_node* rollout_base = curr;
    std::shared_ptr<AST> ast = build_ast_upward(rollout_base);

    std::queue<std::shared_ptr<AST>> targets;
    set_targets_from_ast(ast, targets);

    auto size = ast->get_size();
    auto num_unconnected = ast->get_num_unconnected();

    while (!targets.empty()) {
      // same type of logic as in add_actions(). don't add nodes to the AST which will
      // cause the AST to be larger than max depth (max size)
      auto max_child_arity = depth_limit_ - (size + num_unconnected);
      std::shared_ptr<AST> targ = targets.front();
      // add actions randomly
      std::shared_ptr<AST> child = targ->add_child(get_random_action(max_child_arity));
      size++;
      num_unconnected += child->vacancy() - 1;
      if (!child->is_terminal()) {
        targets.push(child); 
      }
      if (targ->is_full()) {
        targets.pop();
      }
    }
    return score(ast);
  }

  /**
   * @brief A recursive method for generating a graph viz representation for
   * the entire MCTS tree
   *
   * @return the graph viz string representation
   */
  template <class MAB, class ScoreFn>
  std::string MCTS<MAB, ScoreFn>::to_gv() const {
    std::stringstream ss;
    ss << "digraph {" << std::endl;
    ss << root_.to_gv();
    ss << "}" << std::endl;
    return ss.str();
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
  template <class MAB, class ScoreFn>
  int MCTS<MAB, ScoreFn>::get_random(int lower, int upper) {
    std::uniform_int_distribution<std::mt19937::result_type> dist(lower, upper);
    return dist(rng_); 
  }

  /**
   * @brief a simple getter for the MCTS class' symbol table
   *
   * @return a non-const reference to the MCTS class' symbol table
   */
  template <class MAB, class ScoreFn>
  dataset& MCTS<MAB, ScoreFn>::dataset() {
    return dataset_;
  }

  /**
   * @brief Returns the AST which this MCTS has currently
   * decided is optimal
   *
   * @return a shared pointer to an AST
   */
  template <class MAB, class ScoreFn>
  std::shared_ptr<AST> MCTS<MAB, ScoreFn>::build_result() {
    return build_ast_upward(curr_);
  }

  /**
   * @brief calls the multi armed bandit function (default UCB1) and returns the result
   * @param Args a parameter pack to be forwarded to UCB1/whatever
   * @return the return type of UCB1/whatever
   */
  template <class MAB, class ScoreFn>
  double MCTS<MAB, ScoreFn>::multi_armed_bandit(double child_val, int child_n, int parent_n) {
    return mab_fn_(child_val, child_n, parent_n);   
  }

  /**
   * @brief calls the score function (default mean squared error) and returns the result
   * @param ast the ast to be used for scoring
   * @return the score
   */
  template <class MAB, class ScoreFn>
  double MCTS<MAB, ScoreFn>::score(std::shared_ptr<AST>& ast) {
    return score_fn_(dataset_, ast);
  }
}

#endif
