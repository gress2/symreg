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
  if (!gap) {
    gap = 1;
  }
  dataset ds;
  for (int i = min; i < max; i += gap) {
    ds.x.push_back(i);
    ds.y.push_back(mapped_lambda(i));
  }
  return ds;
}

}
#endif
