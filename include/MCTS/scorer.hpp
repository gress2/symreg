#pragma once

namespace symreg
{
namespace scorer 
{

/**
 * @brief a scorer object interface
 */
class scorer {
  public:
    virtual double score(double, int, int) = 0;
};

/**
 * @brief a UCB1 scorer
 */
class UCB1 : public scorer {
  public:
    double score(double, int, int);
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
} // symreg
