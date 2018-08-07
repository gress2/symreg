#pragma once

namespace symreg
{
namespace MCTS
{
namespace scorer 
{

// SCORER INTERFACE

template <class T>
class scorer {
  public:
    double score(double, int, int);
};

template <class T>
double scorer<T>::score(double child_val, int child_n, int parent_n) {
  return static_cast<T*>(this)->calc_score(child_val, child_n, parent_n);
}

// UCB1

class UCB1 : public scorer<UCB1> {
  public:
    double calc_score(double, int, int);
};

double UCB1::calc_score(double child_val, int child_n, int parent_n) {
  return child_val + sqrt(2 * log(parent_n) / child_n);
}

} // scorer
} // MCTS
} // symreg
