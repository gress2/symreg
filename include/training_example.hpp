#pragma once

#include <vector>

namespace symreg
{

struct training_example {
  std::string state;
  std::vector<double> pi;
  double reward;
};

using training_examples = std::vector<training_example>; 

}
