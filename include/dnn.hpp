#ifndef SYMREG_MCTS_DNN_HPP_
#define SYMREG_MCTS_DNN_HPP_

#include <iostream>
#include <dlib/dnn.h>
#include "training_example.hpp"

namespace symreg
{

using values_type = std::vector<double>;
using policy_type = std::vector<double>;

class DNN {
  private:

  public:
    template <class State>
    std::pair<values_type, policy_type> operator()(State&&); 
    void train(training_examples&);
};

template <class State>
std::pair<values_type, policy_type> DNN::operator()(State&& state) {
  return std::make_pair(values_type{}, policy_type{});
}

void DNN::train(training_examples& examples) {

}


}

#endif