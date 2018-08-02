#ifndef SYMREG_MCTS_DNN_HPP_
#define SYMREG_MCTS_DNN_HPP_

#include <iostream>
#include <dlib/dnn.h>

namespace symreg
{
  using values_type = std::vector<double>;
  using policy_type = std::vector<double>;
  
  class DNN {
    private:

    public:
      std::pair<values_type, policy_type> operator()(std::vector<int>&& state) {
        return std::make_pair(values_type{}, policy_type{});
      } 
  };
}

#endif
