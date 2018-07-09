#ifndef SYMREG_DATASET_HPP_
#define SYMREG_DATASET_HPP_

#include <vector>

namespace symreg 
{

struct dataset {
  std::vector<std::vector<double>> x;
  std::vector<double> y;
};

}

#endif
