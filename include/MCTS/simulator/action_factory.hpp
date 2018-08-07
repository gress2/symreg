#pragma once

namespace symreg
{
namespace MCTS
{
namespace simulator
{
  using AST = brick::AST::AST;

  class action_factory {
    private:
      std::vector<std::unique_ptr<brick::AST::node>> binary_set_;
      std::vector<std::unique_ptr<brick::AST::node>> unary_set_;
      std::vector<std::unique_ptr<brick::AST::node>> function_set_;
      std::vector<std::unique_ptr<brick::AST::node>> var_set_;
      std::vector<std::unique_ptr<brick::AST::node>> scalar_set_;
    public:
      action_factory();
      action_factory(symreg::util::config);
      std::vector<std::unique_ptr<brick::AST::node>> get_set(int) const;
      std::unique_ptr<brick::AST::node> get_random(int) const;
      int max_set_size() const;
  }; 

  action_factory::action_factory() 
    : binary_set_({
        std::make_unique<brick::AST::addition_node>(),
        std::make_unique<brick::AST::subtraction_node>(),
        std::make_unique<brick::AST::multiplication_node>(),
        std::make_unique<brick::AST::division_node>(),
      }),
      unary_set_({
        std::make_unique<brick::AST::negate_node>(),
      }),
      function_set_({

      }),
      var_set_({
        std::make_unique<brick::AST::id_node>("_x0"),
      }),
      scalar_set_({
        std::make_unique<brick::AST::number_node>(2),
        std::make_unique<brick::AST::number_node>(3),
        std::make_unique<brick::AST::number_node>(4)
      })
  {}

  action_factory::action_factory(symreg::util::config cfg) {
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

    auto copy_from = [](std::vector<std::unique_ptr<brick::AST::node>>& src,
        std::vector<std::unique_ptr<brick::AST::node>>& dest) {
      for (auto& elem : src) {
        dest.push_back(std::unique_ptr<brick::AST::node>(elem->copy()));
      } 
    };

    if (max_arity >= 2) {
      copy_from(binary_set_, actions);
    }

    if (max_arity >= 1) {
      copy_from(unary_set_, actions);
      copy_from(function_set_, actions);
    }

    copy_from(var_set_, actions);
    copy_from(id_set_, actions);

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
    int random = util::get_random_int(0, action_set.size() - 1, MCTS::mt);
    return std::move(action_set[random]);
  }

  int action_factory::max_set_size() const {
    return binary_set_.size() + unary_set_.size() + function_set_.size() +
     var_set_.size() + id_set_.size(); 
  }
}
}
}
