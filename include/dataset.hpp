#pragma once

#include "brick.hpp"
#include "util.hpp"

#include <vector>

namespace symreg 
{

/**
 * @brief a simple container for single dimension x and y data
 * where f(x) = y for some f
 */
struct dataset {
  std::vector<double> x;
  std::vector<double> y;
};

/**
 * @brief maps a lambda (f) over a range of integers x to produce y
 * @param mapped_lambda the function, f, to apply to each x
 * @param the size of the vectors x and y, i.e. the number of x values to
 * produce in the range
 * @param min the minimum x value in the range
 * @param max the maximum x value in the range
 * @return a dataset of xs and their corresponding lambda outputs
 */
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

/**
 * @brief a method for generating datasets based on a util::config
 *
 * Extracts necessary properties from the config and calls the other overload
 * with the appropriate parameters
 *
 * @return a dataset of xs and their corresponding lambda outputs
 */
dataset generate_dataset(symreg::util::config& cfg) {
  dataset ds;
  std::string fn = cfg.get<std::string>("dataset.function");
  std::unique_ptr<brick::AST::AST> ast = brick::AST::parse(fn);
  auto lambda = [&] (int x) {
    return ast->eval(x);
  };
  int n = cfg.get<int>("dataset.n");
  int min = cfg.get<int>("dataset.xmin");
  int max = cfg.get<int>("dataset.xmax");
  return generate_dataset(lambda, n, min, max);
}

} // end symreg
