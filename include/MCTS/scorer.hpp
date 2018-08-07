#pragma once

namespace symreg
{
namespace scorer 
{

// SCORER INTERFACE

class scorer {
  public:
    virtual double score(double, int, int) = 0;
};

// UCB1

class UCB1 : public scorer {
  public:
    double score(double, int, int);
};

double UCB1::score(double child_val, int child_n, int parent_n) {
  return child_val + sqrt(2 * log(parent_n) / child_n);
}

std::shared_ptr<scorer> get(std::string scorer_str) {
  if (scorer_str == "UCB1") {
    return std::make_shared<UCB1>();
  } else {
    return std::make_shared<UCB1>();
  }
}

} // scorer
} // symreg
