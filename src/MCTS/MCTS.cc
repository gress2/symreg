#include <iostream>
#include <map>
#include <queue>

#include "MCTS/MCTS.hpp"

namespace symreg
{
  /**
   * @brief MCTS constructor
   * 
   * A root search node is constructed using a unary + operator.
   * This node is automatically expanded with all possible actions.
   * The curr_ pointer is initialized to point at this search node.
   * Additionally, the classes random number generator instance is seeded.
   */
  MCTS::MCTS()
    : root_(search_node(std::make_unique<brick::AST::posit_node>())),
      curr_(&root_)
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
  void MCTS::simulate() {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    for (std::size_t i = 0; i < num_simulations_; i++) {
      search_node* leaf = choose_leaf();
      if (leaf->visited()) {
        if (add_actions(leaf)) {
          leaf = &(leaf->get_children()[0]);
        } else {
          std::cout << "no actions added to this leaf" << std::endl;
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
   * 
   * @param n number determining how many time we wish to loop
   */
  void MCTS::iterate(std::size_t n) {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    for (std::size_t i = 0; i < n; i++) {
      std::cout << "Iteration: " << i << std::endl;
      simulate();
      make_move();
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
   */
  void MCTS::make_move() {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    curr_ = curr_->max_UCB1();
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
  search_node* MCTS::choose_leaf() {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    search_node* leaf = curr_;
    while (!leaf->is_leaf_node()) {
      leaf = leaf->max_UCB1();
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
  void MCTS::backprop(double value, search_node* curr) {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
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
  std::vector<search_node*> MCTS::get_up_link_targets(search_node* curr) {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    std::map<search_node*, int> targets; 
    search_node* ancestor = curr;
    while (ancestor != nullptr) {
      if (!ancestor->is_terminal()) {
        if (!targets.count(ancestor)) {
          targets[ancestor] = 0;
        }
      }
      if (ancestor->up_link()) {
        targets[ancestor->up_link()] += 1;
      }
      ancestor = ancestor->parent();
    }
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
  search_node* MCTS::get_earliest_up_link_target(search_node* curr) {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    auto targets = get_up_link_targets(curr);
    if (targets.empty()) {
      return nullptr;
    } else {
      return targets.front();
    }
  }

  /**
   * @brief returns a unique pointer to a randomly chosen AST node type
   *
   * A random number is generated and used to return the corresponding node type
   *
   * @return a unique pointer to a randomly chosen node type
   */
  std::unique_ptr<brick::AST::node> MCTS::get_random_action() {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    int r = get_random(0, 4);
    if (r == 0) {
      return std::make_unique<brick::AST::number_node>(3);
    } else if (r == 1) {
      return std::make_unique<brick::AST::addition_node>();
    } else if (r == 2) {
      return std::make_unique<brick::AST::multiplication_node>();
    } else if (r == 3) { 
      return std::make_unique<brick::AST::id_node>("z");
    } else {
      return std::make_unique<brick::AST::id_node>("y");
    }
  }

  /**
   * @brief returns a vector of unique pointers to all possible node types
   *
   * a vector is created and unique pointers to all AST node types are pushed
   * onto the vector
   *
   * @return a vector of unique pointers for all possible node types
   */
  std::vector<std::unique_ptr<brick::AST::node>> MCTS::get_action_set() {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    std::vector<std::unique_ptr<brick::AST::node>> actions;
    actions.push_back(std::make_unique<brick::AST::addition_node>());
    actions.push_back(std::make_unique<brick::AST::number_node>(3));
    actions.push_back(std::make_unique<brick::AST::multiplication_node>());
    return actions; 
  }

  /**
   * @brief Expansion, i.e., given a search node, attaches children nodes for all possible moves
   * from the node. 
   *
   * starting from curr, the tree is searched upward to find nodes which have
   * available slots for children to be attached. for each of the nodes found,
   * all possible actions are attached to curr and up-linked to the the node. curr may only
   * be expanded while there are still possible moves to be made. 
   *
   * @param curr the node to be expanded
   * @return a boolean denoting whether or not the node was expanded
   */
  bool MCTS::add_actions(search_node* curr) {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    std::vector<search_node*> targets = get_up_link_targets(curr);
    if (targets.empty()) {
      return false;
    }
    std::vector<std::unique_ptr<brick::AST::node>> actions = get_action_set();
    for (search_node* targ : targets) {
      for (auto it = actions.begin(); it != actions.end();) {
        auto child = curr->add_child(std::move(*it));
        child->set_parent(curr);
        child->set_up_link(targ);
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
  std::shared_ptr<brick::AST::AST> MCTS::build_ast_upward(search_node* bottom) {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    search_node* cur = bottom;
    search_node* root = &root_;
    std::map<search_node*, std::shared_ptr<brick::AST::AST>> search_to_ast;

    while (cur != root) {
      search_to_ast[cur] = std::make_shared<brick::AST::AST>
        (std::unique_ptr<brick::AST::node>(cur->ast_node()->clone()));
      cur = cur->parent();
    }

    search_to_ast[root] = std::make_shared<brick::AST::AST>
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
  void set_targets_from_ast(std::shared_ptr<brick::AST::AST> ast, 
    std::queue<std::shared_ptr<brick::AST::AST>>& targets) {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    if (!ast->is_full()) {
      targets.push(ast);
    }
    for (std::shared_ptr<brick::AST::AST>& child : ast->get_children()) {
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
   *
   * Design decision: FIFO method of appending random nodes -- does it matter?
   *
   * @param curr the node to rollout from
   * @return the value of our randomly rolled out AST
   */
  double MCTS::rollout(search_node* curr) {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    search_node* rollout_base = curr;
    std::shared_ptr<brick::AST::AST> ast = build_ast_upward(rollout_base);
    std::queue<std::shared_ptr<brick::AST::AST>> targets;   
    set_targets_from_ast(ast, targets);
    while (!targets.empty()) {
      std::shared_ptr<brick::AST::AST> targ = targets.front();
      std::shared_ptr<brick::AST::AST> child = targ->add_child(get_random_action());
      if (!child->is_terminal()) {
        targets.push(child); 
      }
      if (targ->is_full()) {
        targets.pop();
      }
    }
    return ast->eval(&symbol_table_);
  }

  /**
   * @brief A recursive method for generating a graph viz representation for
   * the entire MCTS tree
   *
   * @return the graph viz string representation
   */
  std::string MCTS::to_gv() const {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
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
  int MCTS::get_random(int lower, int upper) {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    std::uniform_int_distribution<std::mt19937::result_type> dist(lower, upper);
    return dist(rng_); 
  }

  /**
   * @brief a simple getter for the MCTS class' symbol table
   *
   * @return a non-const reference to the MCTS class' symbol table
   */
  std::unordered_map<std::string, double>& MCTS::symbol_table() {
    #ifdef DEBUG 
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    #endif
    return symbol_table_;
  }
}
