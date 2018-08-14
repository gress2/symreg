#pragma once

namespace symreg
{
namespace MCTS
{
namespace scorer 
{

/**
 * @brief a scorer object interface
 */
class scorer {
  public:
    virtual double score(double, int, int) = 0;
    virtual double score(double, int, int, double) = 0;
};

/**
 * @brief a UCB1 scorer
 */
class UCB1 : public scorer {
  public:
    double score(double, int, int);
    double score(double, int, int, double);
};

/**
 * @brief calculates the UCB1 value of a search node
 * @param child_val the q value of the node in question
 * @param child_n the visit count of the node in question
 * @param parent_n the visit count of the nodes parent
 * @return the UCB1 value
 */
double UCB1::score(double child_val, int child_n, int parent_n) {
  return child_val + sqrt(2 * log(parent_n) / child_n);
}

double UCB1::score(double child_val, int child_n, int parent_n, double avg_child_val) {
  avg_child_val = std::max(sqrt(2), avg_child_val);
  return child_val + avg_child_val * sqrt(log(parent_n) / child_n); 
}

/**
 * @brief gets a scorer instance given its string representation
 * @param scorer_str the string representation of a scorer
 * @return a scorer instance
 */
std::shared_ptr<scorer> get(std::string scorer_str) {
  if (scorer_str == "UCB1") {
    return std::make_shared<UCB1>();
  } else {
    return std::make_shared<UCB1>();
  }
}

} // scorer
} // MCTS
} // symreg
