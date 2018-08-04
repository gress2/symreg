#ifndef SYMREG_POLICY_ITERATION_DRIVER_HPP_
#define SYMREG_POLICY_ITERATION_DRIVER_HPP_

#include "training_example.hpp"

namespace symreg
{

template <class NeuralNet, class TreeSearch>
class policy_iteration_driver {
  private:
    NeuralNet& nn_;
    TreeSearch& mcts_;
    int num_iterations_ = 10;
    int num_episodes_ = 10;
    training_examples examples_;
  public:
    policy_iteration_driver(NeuralNet&, TreeSearch&);
    void iterate();
};

template <class NeuralNet, class TreeSearch>
policy_iteration_driver<NeuralNet, TreeSearch>::policy_iteration_driver(NeuralNet& nn, TreeSearch& mcts) 
  : nn_(nn), mcts_(mcts)
{}

template <class NeuralNet, class TreeSearch>
void policy_iteration_driver<NeuralNet, TreeSearch>::iterate() {
  for (int i = 0; i < num_iterations_; i++) {
    for (int j = 0; j < num_episodes_; j++) {
      mcts_.reset();
      mcts_.iterate();
      auto new_examples = mcts_.get_training_examples();
      std::cout << mcts_.get_result()->to_string() << std::endl;
      examples_.insert(examples_.end(), new_examples.begin(), new_examples.end()); 
    }
    nn_.train(examples_);
  }
}

}
#endif

