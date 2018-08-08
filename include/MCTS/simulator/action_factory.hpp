#pragma once

namespace symreg
{
namespace MCTS
{
namespace simulator
{
  using AST = brick::AST::AST;

  /**
   * produces AST nodes to be appended to MCTS trees. capable
   * of returning a vector of all valid AST nodes which can be added
   * as children to a given search_node. also can be used to just
   * get a random AST node for use in random rollouts
   */
  class action_factory {
    private:
      std::vector<std::unique_ptr<brick::AST::node>> binary_set_;
      std::vector<std::unique_ptr<brick::AST::node>> unary_set_;
      std::vector<std::unique_ptr<brick::AST::node>> function_set_;
      std::vector<std::unique_ptr<brick::AST::node>> var_set_;
      std::vector<std::unique_ptr<brick::AST::node>> scalar_set_;
    public:
      action_factory();
      action_factory(symreg::util::config&);
      action_factory(const action_factory&);
      std::vector<std::unique_ptr<brick::AST::node>> get_set(int) const;
      std::unique_ptr<brick::AST::node> get_random(int) const;
      int max_set_size() const;
  }; 

  /**
   * @brief a default action_factory constructor
   */
  action_factory::action_factory() {
    // for whatever reason we can't initialize the vectors with initialization lists
    binary_set_.push_back(std::make_unique<brick::AST::addition_node>());
    binary_set_.push_back(std::make_unique<brick::AST::subtraction_node>());
    binary_set_.push_back(std::make_unique<brick::AST::multiplication_node>());
    binary_set_.push_back(std::make_unique<brick::AST::division_node>());
    unary_set_.push_back(std::make_unique<brick::AST::negate_node>());
    var_set_.push_back(std::make_unique<brick::AST::id_node>("_x0"));
    scalar_set_.push_back(std::make_unique<brick::AST::number_node>(2));
    scalar_set_.push_back(std::make_unique<brick::AST::number_node>(3));
    scalar_set_.push_back(std::make_unique<brick::AST::number_node>(4));
  }

  /**
   * @brief builds action_factory according to the settings specified in .toml config
   * @param cfg a wrapper around a .toml config
   */
  action_factory::action_factory(util::config& cfg) {
    std::vector<std::string> binary = cfg.get_vector<std::string>("actions.binary");
    std::vector<std::string> unary = cfg.get_vector<std::string>("actions.unary");
    std::vector<std::string> functions = cfg.get_vector<std::string>("actions.functions");
    std::vector<std::string> vars = cfg.get_vector<std::string>("actions.vars");
    int scalar_min = cfg.get<int>("actions.scalar_min");
    int scalar_max = cfg.get<int>("actions.scalar_max");

    for (auto& elem : binary) {
      if (elem == "addition") {
        binary_set_.push_back(std::make_unique<brick::AST::addition_node>());
      } else if (elem == "subtraction") {
        binary_set_.push_back(std::make_unique<brick::AST::subtraction_node>());
      } else if (elem == "multiplication") {
        binary_set_.push_back(std::make_unique<brick::AST::multiplication_node>());
      } else if (elem == "division") {
        binary_set_.push_back(std::make_unique<brick::AST::division_node>());
      }
    }

    for (auto& elem : unary) {
      if (elem == "posit") {
        unary_set_.push_back(std::make_unique<brick::AST::posit_node>());
      } else if (elem == "negate") {
        unary_set_.push_back(std::make_unique<brick::AST::negate_node>());
      }
    }

    for (auto& elem : functions) {
      
    }

    for (auto& elem : vars) {
      var_set_.push_back(std::make_unique<brick::AST::id_node>(elem));
    }

    for (int i = scalar_min; i <= scalar_max; i++) {
      scalar_set_.push_back(std::make_unique<brick::AST::number_node>(i));
    }
  }

  /**
   * @brief copies an actions set from a source to a destination vector.
   *
   * Invokes the (virtually dispatched) copy method of the ASTs in the source
   * vector. the copy method is the same as the clone method, however, it also
   * assigns a new node_id to the node, thus avoiding problems when we go to
   * create graphviz graphics of MCTS 
   *
   * @param src a reference to the vector of AST nodes to copy from
   * @param dest a reference to the vector of AST nodes to copy to
   */
  void copy_from(const std::vector<std::unique_ptr<brick::AST::node>>& src,
      std::vector<std::unique_ptr<brick::AST::node>>& dest) {
    for (auto& elem : src) {
      dest.push_back(std::unique_ptr<brick::AST::node>(elem->copy()));
    }
  }

  action_factory::action_factory(const action_factory& other) {
    copy_from(other.binary_set_, this->binary_set_);
    copy_from(other.unary_set_, this->unary_set_);
    copy_from(other.function_set_, this->function_set_);
    copy_from(other.var_set_, this->var_set_);
    copy_from(other.scalar_set_, this->scalar_set_);
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
  std::vector<std::unique_ptr<brick::AST::node>> action_factory::get_set(int max_arity) const {
    std::vector<std::unique_ptr<brick::AST::node>> actions;

    if (max_arity >= 2) {
      copy_from(binary_set_, actions);
    }

    if (max_arity >= 1) {
      copy_from(unary_set_, actions);
      copy_from(function_set_, actions);
    }

    copy_from(var_set_, actions);
    copy_from(scalar_set_, actions);

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
  std::unique_ptr<brick::AST::node> action_factory::get_random(int max_arity) const {
    std::vector<std::unique_ptr<brick::AST::node>> action_set = get_set(max_arity);
    int random = util::get_random_int(0, action_set.size() - 1, symreg::mt);
    return std::move(action_set[random]);
  }

  /**
   * @brief essentially gets the max number of children a search node may have
   * given this action factory
   * @return the total number of distinct AST nodes this action factory can return
   */
  int action_factory::max_set_size() const {
    return binary_set_.size() + unary_set_.size() + function_set_.size() +
     var_set_.size() + scalar_set_.size(); 
  }
}
}
}
