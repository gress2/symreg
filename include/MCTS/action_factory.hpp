#ifndef SYMREG_MCTS_ACTION_FACTORY_HPP_
#define SYMREG_MCTS_ACTION_FACTORY_HPP_

namespace symreg
{
namespace MCTS
{
namespace simulator
{
  using AST = brick::AST::AST;

  class action_factory {
    private:
      std::mt19937& mt_;
      int num_dims_;
    public:
      action_factory(std::mt19937&, int);
      std::vector<std::unique_ptr<brick::AST::node>> get_set(int);
      std::unique_ptr<brick::AST::node> get_random(int);
  }; 

  action_factory::action_factory(std::mt19937& mt, int num_dims) 
    : mt_(mt), num_dims_(num_dims)
  {
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
  std::vector<std::unique_ptr<brick::AST::node>> action_factory::get_set(int max_arity) {
    std::vector<std::unique_ptr<brick::AST::node>> actions;
    // binary operators
    if (max_arity >= 2) {
      actions.push_back(std::make_unique<brick::AST::addition_node>());
      actions.push_back(std::make_unique<brick::AST::multiplication_node>());
    }
    // unary operators
    if (max_arity >= 1) {
    }
    // terminals
    for (int a = 1; a < 3; a++) {
      actions.push_back(std::make_unique<brick::AST::number_node>(a));
    }

    for (int i = 0; i < num_dims_; i++) {
      actions.push_back(std::make_unique<brick::AST::id_node>("_x" + std::to_string(i)));
    }

    return actions;
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
  std::unique_ptr<brick::AST::node> action_factory::get_random(int max_arity) {
    std::vector<std::unique_ptr<brick::AST::node>> action_set = get_set(max_arity);
    int random = util::get_random_int(0, action_set.size() - 1, mt_);
    return std::move(action_set[random]);
  }
}
}
}

#endif
