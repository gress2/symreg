#pragma once

#include <vector>

namespace symreg
{
/**
 * @brief a plain old data structure for
 * MCTS produced, regressor consumed training examples
 */
struct training_example {
  std::string state;
  std::vector<double> pi;
  double reward;
};

using training_examples = std::vector<training_example>; 

}
