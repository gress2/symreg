#ifndef SYMREG_DATASET_HPP_
#define SYMREG_DATASET_HPP_

#include <vector>

namespace symreg 
{

struct dataset {
  std::vector<double> x;
  std::vector<double> y;
};

template <class T>
dataset generate_dataset(T mapped_lambda, int n, int min, int max) {
  int gap = (max - min) / n;
  dataset ds;
  for (int i = min; i < max; i += gap) {
    std::cout << "(" << i << ", " << mapped_lambda(i) << ")" << std::endl;
    ds.x.push_back(i);
    ds.y.push_back(mapped_lambda(i));
  }
  return ds;
}

}
#endif
