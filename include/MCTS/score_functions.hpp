#ifndef SYMREG_MCTS_SCORE_FUNCTIONS_HPP_
#define SYMREG_MCTS_SCORE_FUNCTIONS_HPP_

namespace symreg
{
namespace MCTS
{
namespace score 
{

static auto UCB1 = [](double child_val, int child_n, int parent_n) { 
  return child_val + sqrt(2 * log(parent_n) / child_n); 
};

}
}
}

#endif
