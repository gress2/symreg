#pragma once

#include "brick.hpp"
#include "util.hpp"

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
